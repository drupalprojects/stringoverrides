<?php

/**
 * @file
 * Drush interface for String Overrides.
 */

/**
 * Implements hook_drush_command().
 */
function stringoverrides_drush_command() {
  $items['stringoverrides-list'] = array(
    'callback' => 'stringoverrides_drush_list',
    'description' => dt('Retrieves all the String Overrides associated with the site.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'arguments' => array(
      'language' => dt('The language for the available overrides. Defaults to "en".'),
    ),
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function stringoverrides_drush_help($section) {
  switch ($section) {
    case 'drush:stringoverrides-list':
      return dt('Retrieves all the String Overrides associated with the site.');
  }
}

/**
 * Drush Callback; Lists all the String Overrides.
 */
function stringoverrides_drush_list($language = 'en') {
  // Load the administration interface for API dependencies.
  module_load_include('inc', 'stringoverrides', 'stringoverrides.admin');
  $strings = array();

  // Retrieve the string overrides from the variables table.
  $words = array(
    FALSE => variable_get("locale_custom_disabled_strings_$language", array()),
    TRUE => variable_get("locale_custom_strings_$language", array())
  );
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

  // Only display the table if there are string overrides.
  if (empty($strings)) {
    drush_print('There are currently no string overrides associated with the given language.');
  }
  else {
    // Sort the strings before displaying them.
    usort($strings, 'stringoverrides_admin_word_sort');

    // Construct and display the table.
    $rows[] = array(dt(''), dt('Original'), dt('Replacement'), dt('Context'));
    foreach ($strings as $string) {
      $rows[] = array(
        $string['enabled'] ? dt('[X]') : dt('[ ]'),
        $string['source'],
        $string['translation'],
        $string['context'],
      );
    }

    // Display in a table.
    drush_print_table($rows, TRUE);
  }
}
