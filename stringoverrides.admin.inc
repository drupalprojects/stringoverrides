<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the String Overrides module.
 */

/**
 * Menu callback for the String Overrides module to display its administration
 */
function stringoverrides_admin($form_state = NULL, $lang = NULL) {
  // See which language we're modifying
  if (empty($lang)) {
    global $language;
    $lang = $language->language;
  }
  $form = array(
    '#cache' => TRUE,
  );
  $form['lang'] = array(
    '#type' => 'hidden',
    '#value' => $lang,
  );
  $form['string'] = array(
    '#theme' => 'stringoverrides_strings',
  );
  // Retrieve the words to display
  $delta = 0;
  if ($form_state['storage']['words']) {
    $words = $form_state['storage']['words'];
  }
  else {
    $words = variable_get('locale_custom_strings_'. $lang, array());
  }
  uksort($words, 'strcasecmp'); // Case insensitive sort by key
  foreach($words as $original => $replacement){
    $form['string'][$delta] = stringoverrides_textbox_combo($delta, TRUE, $original, $replacement);
    $delta++;
  }
  for ($index = 0; $index < 3; $index++) {
    $form['string'][$delta] = stringoverrides_textbox_combo($delta);
    $delta++;
  }
  
  $form['more_strings'] = array(
    '#type' => 'button',
    '#value' => t('More strings'),
    '#description' => t("If the amount of boxes above isn't enough, click here to add more choices."),
    '#weight' => 2,
    '#ahah' => array(
      'path' => 'admin/settings/stringoverrides/js',
      'wrapper' => 'stringoverrides-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
      'progress' => 'none',
    ),
  );
  $form['save'] = array(
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
} // function stringoverrides_admin_submit

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
    '#type' => 'textarea',
    '#default_value' => $original,
    '#rows' => 1,
    '#parents' => array('string', $delta, 'original'),
  );
  $form['replacement'] = array(
    '#type' => 'textarea',
    '#default_value' => $replacement,
    '#rows' => 1,
    '#parents' => array('string', $delta, 'replacement'),
  );
  return $form;
} // function stringoverrides_textbox_combo

/**
 * Theme the enabled box and the two text box strings
 */
function theme_stringoverrides_strings($form) {
  drupal_add_css(drupal_get_path('module', 'stringoverrides') .'/stringoverrides.css', 'module', NULL, NULL, FALSE);
  
  $headers = array(
    theme('table_select_header_cell'),
    t('Original'),
    t('Replacement'),
  );
  $rows = array();
  foreach (element_children($form) as $key) {
    // Build the table row.
    $rows[$key] = array(
      'data' => array(
        array('data' => drupal_render($form[$key]['enabled']), 'class' => 'stringoverrides-enabled'),
        array('data' => drupal_render($form[$key]['original']), 'class' => 'stringoverrides-original'),
        array('data' => drupal_render($form[$key]['replacement']), 'class' => 'stringoverrides-replacement'),
      ),
    );
    // Add any attributes on the element to the row, such as the ahah class.
    if (array_key_exists('#attributes', $form[$key])) {
      $rows[$key] = array_merge($rows[$key], $form[$key]['#attributes']);
    }
  }

  $output  = '';
  $output .= '<div id="stringoverrides-wrapper">';
  $output .= theme('table', $headers, $rows);
  $output .= '</div>';
  $output .= drupal_render($form);
  return $output;
} // function theme_stringoverrides_strings

/**
 * Menu callback for the String Overrides module to display a new string override
 */
function stringoverrides_js() {
  $delta = count($_POST['string']);

  // Build our new form element.
  $form_element = stringoverrides_textbox_combo($delta);
  drupal_alter('form', $form_element, array(), 'stringoverrides_js');

  // Build the new form.
  $form_state = array('submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  // Add the new element to the stored form. Without adding the element to the
  // form, Drupal is not aware of this new elements existence and will not
  // process it. We retreive the cached form, add the element, and resave.
  $form = form_get_cache($form_build_id, $form_state);
  $form['string'][$delta] = $form_element;
  form_set_cache($form_build_id, $form, $form_state);
  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
  );

  // Rebuild the form.
  $form = form_builder('stringoverrides_admin', $form, $form_state);

  // Render the new output.
  $string_form = $form['string'];
  $string_form[$delta]['enabled']['#value'] = TRUE;
  $string_form[$delta]['#attributes']['class'] = empty($string_form[$delta]['#attributes']['class']) ? 'ahah-new-content' : $string_form[$delta]['#attributes']['class'] .' ahah-new-content';
  $output = theme('status_messages') . drupal_render($string_form);

  drupal_json(array('status' => TRUE, 'data' => $output));
} // function stringoverrides_js



/**
 * Menu callback for the String Overrides module to display the import administration
 */
function stringoverrides_admin_import() {
  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $languages = module_exists('locale') ? locale_language_list() : array('en' => t('English'));
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('File'),
    '#description' => t('Attach your *.po file here to import the string overrides.'),
  );
  $form['lang'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#description' => t('Which language to import the overrides to.'),
    '#options' => $languages,
  );
  $form['import'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#weight' => 3,
  );
  return $form;
} // function stringoverrides_admin_import


/**
 * Triggered when the user imports data
 */
function stringoverrides_admin_import_submit($form, &$form_state) {
  // Check if the file uploaded correctly
  if ($file = file_save_upload('file')) {
    $overrides = array();
  
    // Start reading the file
    $handle = @fopen($file->filepath, "r");
    if ($handle) {
      $currentoverride = NULL;
      $currentstring = NULL;
      $operation = 'msgstr';
      
      // Loop through the whole file
      while (!feof($handle)) {
        // Retrieve a single line
        $buffer = trim(fgets($handle));
        // Skip empty or comment lines
        if (empty($buffer) || $buffer[0] == '#') {
          continue;
        }
        // Continued string
        if ($buffer[0] == '"') {
          // See what we're reading in
          $string = trim(substr($buffer, 1, -1));
          if ($operation == 'msgstr' && !empty($string)) {
            $currentstring .= $string;
          }
          else {
            $currentoverride .= $string;
          }
        }
        else if (substr($buffer, 0, 6) == 'msgstr') {
          // Retrieve the override string
          $operation = 'msgstr';
          $currentstring = substr($buffer, 8, -1);
        }
        else if (substr($buffer, 0, 5) == 'msgid') { // New string
          // Save old string
          if (!empty($currentstring) && !empty($currentoverride)) {
            $overrides[$currentoverride] = $currentstring;
            $currentoverride = $currentstring = '';
          }
          // Read what's next
          $operation = 'msgid';
          $currentoverride = substr($buffer, 7, -1);
        }
      }
      
      // Clean up and save the imported data
      fclose($handle);
      file_delete($file->filepath);
      variable_set('locale_custom_strings_'. $form_state['values']['lang'], $overrides);
      drupal_set_message(t('The overrides have been imported.'));
    }
  }
  else {
    form_set_error('file', t('A file to import is required.'));
  }
} // function stringoverrides_admin_import_submit

/**
 * Ability to export a *.po file.
 */
function stringoverrides_admin_export() {
  $form = array();
  $languages = module_exists('locale') ? locale_language_list() : array('en' => t('English'));
  $form['lang'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#description' => t('The language you would like to export.'),
    '#options' => $languages,
    '#required' => TRUE
  );
  $form['export'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
    '#weight' => 3,
  );
  $form['#submit'][] = 'stringoverrides_admin_export_submit';
  return $form;
} // function stringoverrides_admin_export

/**
 * Submit-handler for stringoverrides_admin_export.
 */
function stringoverrides_admin_export_submit($form, &$form_state) {
  drupal_set_header('Content-Type: text/plain');
  drupal_set_header('Content-Disposition: attachment; filename: "stringoverrides.po"');
  $export = stringoverrides_admin_export_text($form_state['values']['lang']);
  drupal_set_header('Content-Length: '. strlen($export));
  echo $export;
} // stringoverrides_admin_export_submit

/**
 * Returns the exported *.po text from the given language.
 */
function stringoverrides_admin_export_text($lang = 'en') {
  $export = '# String Overrides Export';
  $overrides = variable_get('locale_custom_strings_'. $lang, array());
  foreach ($overrides as $override => $string) {
    $export .= 'msgid "'. $override ."\"\nmsgstr \"$string\"\n";
  }
  return $export;
} // function stringoverrides_admin_export_text
