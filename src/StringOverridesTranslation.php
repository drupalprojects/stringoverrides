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
   * {@inheritdoc}
   */
  protected function getLanguage($langcode) {
    $cid = 'stringoverides:translation_for_' . $langcode;

    if ($cache = \Drupal::cache()->get($cid)) {
      return $cache->data;
    }
    else {
      $translations = [];
      // Drupal configuration array structure is different from translations
      // array structure, lets transform configuration array.
      $config = \Drupal::config('stringoverrides.string_override.' . $langcode);
      $contexts = $config->get('contexts');
      if (!empty($contexts)) {
        foreach ($contexts as $context) {
          foreach ($context['translations'] as $word) {
            $translations[$context['context']][$word['source']] = $word['translation'];
          }
        }
      }
      \Drupal::cache()->set($cid, $translations);
      return $translations;
    }
  }

}
