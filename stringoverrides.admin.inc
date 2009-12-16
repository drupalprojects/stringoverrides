<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the String Overrides module.
 */

/**
 * Menu callback for the String Overrides module to display its administration
 */
function stringoverrides_admin($form, $form_state, $lang = NULL) {
  // See which language we're modifying
  if (empty($lang)) {
    global $language;
    $lang = $language->language;
  }

  // Setup the form
  $form['#cache'] = TRUE;
  $form['#attached']['css'][drupal_get_path('module', 'stringoverrides') . '/stringoverrides.css'] = array();
  
  $form['lang'] = array(
    '#type' => 'hidden',
    '#value' => $lang,
  );
  $form['string'] = array(
    '#tree' => TRUE,
    '#prefix' => '<div id="stringoverrides-wrapper">',
    '#suffix' => '</div>',
    '#theme' => 'stringoverrides_strings',
  );

  $words = array(
    FALSE => variable_get("locale_custom_disabled_strings_$lang", array()),
    TRUE => variable_get("locale_custom_strings_$lang", array())
  );
  $strings = array();
  foreach ($words as $enabled => $custom_strings) {
    foreach ($custom_strings as $context => $translations) {
      foreach ($translations as $source => $translation) {
        $strings[] = array(
          'enabled' => $enabled,
          'context' => $context,
          'source' => $source,
          'translation' => $translation,
        );
      }
    }
  }

  if (isset($form_state['string_count'])) {
    $string_count = $form_state['string_count'];
  }
  else {
    $string_count = max(3, count($strings));
  }

  // Sort the strings and display them in the form
  usort($strings, 'stringoverrides_admin_word_sort');
  for ($index = 0; $index < $string_count; $index++) {
    if (isset($strings[$index])) {
      $string = $strings[$index];
      $form['string'][$index] = stringoverrides_textbox_combo($index, $string['enabled'], $string['context'], $string['source'], $string['translation']);
    }
    else {
      $form['string'][$index] = stringoverrides_textbox_combo($index, -1);
    }
  }

  // Add the buttons to the form
  $form['more_strings'] = array(
    '#type' => 'submit',
    '#value' => t('Add row'),
    '#description' => t("If the amount of boxes above isn't enough, click here to add more choices."),
    '#weight' => 2,
    '#submit' => array('stringoverrides_more_strings_submit'),
    '#ajax' => array(
      'callback' => 'stringoverrides_ajax',
      'wrapper' => 'stringoverrides-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
      'progress' => 'none',
    ),
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 3,
  );
  $form['remove'] = array(
    '#type' => 'submit',
    '#value' => t('Remove disabled strings'),
    '#weight' => 4,
    '#access' => !empty($strings),
  );
  return $form;
}

/**
 * Triggered when the user submits the administration page
 */
function stringoverrides_admin_submit($form, &$form_state) {
  if (!in_array($form_state['clicked_button']['#id'], array('edit-save', 'edit-remove'))) {
    // Submit the form only for save and remove buttons.
    return;
  }

  // Format the words correctly so that they're put into the database correctly
  $words = array(FALSE => array(), TRUE => array());
  foreach ($form_state['values']['string'] as $index => $string) {
    if (!empty($string['source'])) {
      $context = check_plain($string['context']);
      // Get rid of carriage returns.
      list($source, $translation) = str_replace("\r", '', array($string['source'], $string['translation']));
      $words[$string['enabled']][$context][$source] = $translation;
    }
  }

  // Save into the correct language definition
  $lang = $form['lang']['#value'];
  if (empty($lang)) {
    global $language;
    $lang = $language->language;
  }
  variable_set("locale_custom_strings_$lang", $words[1]);

  // Save the values and display a message to the user depend
  switch ($form_state['clicked_button']['#id']) {
    case 'edit-save':
      variable_set("locale_custom_disabled_strings_$lang", $words[0]);
      drupal_set_message('Your changes have been saved.');
      break;

    case 'edit-remove':
      variable_del("locale_custom_disabled_strings_$lang");
      drupal_set_message('The disabled strings have been removed.');
      break;
  }
}

/**
 * Function to return a textbox combo form
 */
function stringoverrides_textbox_combo($delta = 0, $enabled = TRUE, $context = '', $source = '', $translation = '') {
  $form['#tree'] = TRUE;
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#default_value' => ($enabled == -1) ? TRUE : $enabled,
    '#access' => $enabled != -1, // Have access if it's not a placeholder value
  );
  $form['context'] = array(
    '#type' => 'textfield',
    '#default_value' => $context,
    '#size' => 20,
    '#maxlength' => 255,
  );
  $form['source'] = array(
    '#type' => 'textarea',
    '#default_value' => $source,
    '#rows' => 1,
  );
  $form['translation'] = array(
    '#type' => 'textarea',
    '#default_value' => $translation,
    '#rows' => 1,
  );
  return $form;
}

/**
 * Theme the enabled box and the two text box strings
 */
function theme_stringoverrides_strings($variables) {
  $form = $variables['form'];
  $rows = array();
  foreach (element_children($form) as $key) {
    // Build the table row.
    $rows[$key] = array(
      'data' => array(
        array('data' => drupal_render($form[$key]['enabled']), 'class' => 'stringoverrides-enabled'),
        array('data' => drupal_render($form[$key]['context']), 'class' => 'stringoverrides-context'),
        array('data' => drupal_render($form[$key]['source']), 'class' => 'stringoverrides-source'),
        array('data' => drupal_render($form[$key]['translation']), 'class' => 'stringoverrides-translation'),
      ),
    );
    // Add any attributes on the element to the row, such as the ahah class.
    if (array_key_exists('#attributes', $form[$key])) {
      $rows[$key] = array_merge($rows[$key], $form[$key]['#attributes']);
    }
  }
  $header = array(
    ($form[0]['enabled']['#access']) ? t('Enabled') : NULL,
    t('Context'),
    t('Original'),
    t('Replacement'),
  );

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Menu callback for the String Overrides module to display a new string override
 */
function stringoverrides_ajax($form, $form_state) {
  return $form['string'];
}

/**
 * Submit handler for the "Add row" button.
 */
function stringoverrides_more_strings_submit($form, &$form_state) {
  $form_state['string_count'] = count($form_state['values']['string']) + 1;
}

/**
 * Sorts two words based on their source text.
 */
function stringoverrides_admin_word_sort($word1, $word2) {
  return strcasecmp($word1['source'], $word2['source']);
}
