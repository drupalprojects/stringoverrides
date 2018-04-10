<?php

namespace Drupal\stringoverrides\Plugin\Derivative;

use Drupal\Component\Plugin\Derivative\DeriverBase;

/**
 * Defines dynamic local tasks (menu tabs).
 */
class DynamicLocalTasks extends DeriverBase {

  /**
   * {@inheritdoc}
   */
  public function getDerivativeDefinitions($base_plugin_definition) {
    $languages = \Drupal::languageManager()->getLanguages();
    foreach ($languages as $language_code => $language) {
      $this->derivatives['stringoverrides.translations_form.' . $language_code] = [
        'title' => $language->getName(),
        'base_route' => 'stringoverrides.translations_form',
        'route_name' => 'stringoverrides.translations_form',
        'route_parameters' => ['language' => $language_code],
        'weight' => 100,
      ] + $base_plugin_definition;
    }
    return parent::getDerivativeDefinitions($base_plugin_definition);
  }

}