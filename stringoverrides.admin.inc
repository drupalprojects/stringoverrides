<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the String Overrides module.
 */

/**
 * Menu callback for the String Overrides module to display its administration
 */
function stringoverrides_admin() {
  $form = array();
  $form['string'] = array(
    '#theme' => 'stringoverrides_strings',
  );
  // Retrieve the words to display
  $delta = 0;
  $words = variable_get('locale_custom_strings_en', array());
  uksort($words, 'strcasecmp'); // Case insensitive sort by key
  foreach($words as $original => $replacement){
    $form['string'][$delta] = stringoverrides_textbox_combo($delta, TRUE, $original, $replacement);
    $delta++;
  }
  for ($index = 0; $index < 3; $index++) {
    $form['string'][$delta] = stringoverrides_textbox_combo($delta);
    $delta++;
  }
  
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
function stringoverrides_admin_submit($form_id, $form_values) {
  // Format the words correctly so that they're put into the database correctly
  $words = array();
  foreach ($form_values['string'] as $index => $value) {
    if (!empty($value['original']) && $value['enabled']) {
      $words[$value['original']] = $value['replacement'];
    }
  }
  
  // Save into the English language definition
  variable_set('locale_custom_strings_en', $words);
  
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
    if ($form[$key]['#attributes']) {
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
 * Menu callback for the String Overrides module to display the import administration
 */
function stringoverrides_admin_import() {
  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('Import *.po File'),
    '#description' => t('Attach your *.po file here to import the string overrides.'),
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
function stringoverrides_admin_import_submit($form_id, $form_values) {
  // Check if the file uploaded correctly
  if ($file = file_check_upload('file')) {
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
      variable_set('locale_custom_strings_en', $overrides);
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
  
  $export = '';
  $overrides = variable_get('locale_custom_strings_en', array());
  foreach ($overrides as $override => $string) {
    $export .= 'msgid "'. $override ."\"\nmsgstr \"$string\"\n";
  }
  $form['overrides'] = array(
    '#type' => 'textarea',
    '#title' => t('Export'),
    '#default_value' => $export,
    '#description' => t('This is the generated *.po file.'),
  );
  return $form;
} // function stringoverrides_admin_export
