<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the String Overrides module.
 */

/**
 * Menu callback for the String Overrides module to display its administration
 */
function stringoverrides_admin($form = NULL, $lang = NULL) {
  // See which language we're modifying
  if (empty($lang)) {
    global $language;
    $lang = $language->language;
  }
  $form['lang'] = array(
    '#type' => 'hidden',
    '#value' => $lang,
  );
  $form['stringoverrides_wrapper'] = array(
    '#tree' => FALSE,
    '#weight' => -4,
    '#prefix' => '<div id="stringoverrides-wrapper">',
    '#suffix' => '</div>',
  );
  $form['stringoverrides_wrapper']['string'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="stringoverrides-wrapper-string">',
    '#suffix' => '</div>',
    '#theme' => 'stringoverrides_strings',
  );
  
  // Retrieve the words to display
  $delta = 0;
  $words = variable_get('locale_custom_strings_'. $lang, array());
  uksort($words, 'strcasecmp'); // Case insensitive sort by key
  foreach($words as $original => $replacement){
    $form['stringoverrides_wrapper']['string'][$delta] = stringoverrides_textbox_combo($delta, TRUE, $original, $replacement);
    $delta++;
  }
  for ($index = 0; $index < 3; $index++) {
    $form['stringoverrides_wrapper']['string'][$delta] = stringoverrides_textbox_combo($delta);
    $delta++;
  }
  
  $form['stringoverrides_wrapper']['more_strings'] = array(
    '#type' => 'submit',
    '#value' => t('More strings'),
    '#description' => t("If the amount of boxes above isn't enough, click here to add more choices."),
    '#submit' => array('stringoverrides_more_strings_submit'), // If no javascript action.
    '#weight' => 2,
    '#ahah' => array(
      'path' => 'admin/settings/stringoverrides/js',
      'wrapper' => 'stringoverrides-wrapper-string',
      'method' => 'append',
      'effect' => 'fade',
      'event' => 'click',
    ),
  );
  $form['stringoverrides_wrapper']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 3,
  );
  return $form;
} // function stringoverrides_admin

/**
 * Triggered when the user submits the administration page
 */ 
function stringoverrides_admin_submit($form, &$form_state) {
  // Format the words correctly so that they're put into the database correctly
  $words = array();
  foreach ($form_state['values']['string'] as $index => $value) {
    if (!empty($value['original']) && $value['enabled']) {
      $words[$value['original']] = $value['replacement'];
    }
  }
  
  // Save into the correct language definition
  global $language;
  $lang = $form['lang']['#value'];
  if (empty($lang)) {
    $lang = $language->language;
  }
  variable_set('locale_custom_strings_'. $lang, $words);
  
  // Output a message to the user
  drupal_set_message('Your changes have been saved.');
} // function stringoverrides_admin_submit()

/**
 * Triggered when the user clicks "More strings" and doesn't have javascript
 */
function stringoverrides_more_strings_submit($form, &$form_state) {
  // Save the values
  stringoverrides_admin_submit($form, $form_state);

  // Tell it to create more form elements
  if ($form_state['values']['more_strings']) {
    $form_state['string_count'] = count($form_state['values']['choice']) + 5;
  }
} // function stringoverrides_more_strings_submit()

/**
 * Function to return a textbox combo form
 */ 
function stringoverrides_textbox_combo($delta = 0, $enabled = TRUE, $original = '', $replacement = '') {
  $form = array(
    '#tree' => TRUE,
  );
  $form['enabled'] = array(
    '#type' => 'checkbox', 
    '#default_value' => $enabled,
    '#parents' => array('string', $delta, 'enabled'),
  );
  $form['original'] = array(
    '#type' => 'textfield',
    '#default_value' => $original,
    '#size' => 30,
    '#parents' => array('string', $delta, 'original'),
  );
  $form['replacement'] = array(
    '#type' => 'textfield',
    '#default_value' => $replacement,
    '#size' => 30,
    '#parents' => array('string', $delta, 'replacement'),
  );
  return $form;
} // function stringoverrides_textbox_combo()

/**
 * Theme the enabled box and the two text box strings
 */
function theme_stringoverrides_strings($form) {
  $headers = array(
    theme('table_select_header_cell'),
    t('Original'),
    t('Replacement'),
  );
  $rows = array();
  foreach (element_children($form) as $key) {
    // Build the table row.
    $rows[] = array(
      'data' => array(
        array('data' => drupal_render($form[$key]['enabled']), 'class' => 'stringoverrides-enabled'),
        array('data' => drupal_render($form[$key]['original']), 'class' => 'stringoverrides-original'),
        array('data' => drupal_render($form[$key]['replacement']), 'class' => 'stringoverrides-replacement'),
      ),
    );
  }
  $output = theme('table', $headers, $rows);
  $output .= drupal_render($form);
  return $output;
} // function theme_stringoverrides_strings()

/**
 * Menu callback for the String Overrides module to display a new string override
 */
function stringoverrides_js() {
  $delta = count($_POST['string']);
  
  $form_state = array('submitted' => FALSE);
  $form[$delta] = stringoverrides_textbox_combo($delta);
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  
  $form = form_builder('stringoverrides_form', $form, $form_state);
  $data = drupal_render($form);
  drupal_json(array('status' => TRUE, 'data' => $data));
} // function stringoverrides_js()
