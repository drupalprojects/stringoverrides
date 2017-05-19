<?php

/**
 * @file
 * Contains \Drupal\stringoverrides\StringOverridesTranslation.
 */

namespace Drupal\stringoverrides;

use Drupal\Core\StringTranslation\Translator\StaticTranslation;

/**
 * Provides string overrides.
 */
class StringOverridesTranslation extends StaticTranslation {

  /**
   * Constructs a StringOverridesTranslation object.
   */
  public function __construct() {
    parent::__construct();
  }

  /**
   * {@inheritdoc}
   */
  protected function getLanguage($langcode) {
    // This is just a dummy implementation.
    // @todo Replace this data.
    return array(
      '' => array(
        'Home' => 'House Of The Rising Sun',
        'Content' => 'Woodstock',
        'Structure' => 'Bridge Over Troubled Water',
        'Appearance' => 'Lucy In The Sky With Diamonds',
        'Extend' => 'Stairway To Heaven',
        'Configuration' => 'Private Investigations',
        'People' => 'Suspicious Minds',
        'Reports' => 'Paranoid',
        'Help' => 'Help!',
        'My account' => 'I Me Mine',
        'Log out' => 'Go Now',
      ),
    );
  }

}
