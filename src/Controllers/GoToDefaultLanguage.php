<?php

namespace Drupal\stringoverrides\Controllers;

use Drupal\Core\Controller\ControllerBase;

/**
 * Class GoToDefaultLanguage.
 *
 * @package Drupal\stringoverrides\Controllers
 */
class GoToDefaultLanguage extends ControllerBase {

  /**
   * Redirect user to admin form for default language.
   *
   * @return \Symfony\Component\HttpFoundation\RedirectResponse
   *   Symfony redirect.
   */
  public function go() {
    $language = \Drupal::languageManager()->getDefaultLanguage();
    return $this->redirect('stringoverrides.translations_form', ['language' => $language->getId()]);
  }
}