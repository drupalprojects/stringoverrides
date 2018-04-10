<?php

namespace Drupal\stringoverrides\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Class StringoverridesAdminForm.
 */
class StringoverridesAdminForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'stringoverrides_admin_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, $language = 'default') {
    $strings = $this->getCurrentTranslations($language);

    $form['lang'] = [
      '#type' => 'value',
      '#value' => $language,
    ];

    $form['translations_table'] = [
      '#type' => 'table',
      '#header' => ['Enabled', 'Original', 'Replacement', 'Context'],
      '#title' => $this->t('Translations'),
      '#attributes' => ['id' => 'stringoverrides-wrapper'],
    ];
    $storage = $form_state->getStorage();
    if (empty($storage['number-of-rows'])) {
      $storage['number-of-rows'] = count($strings) + 1;
      $form_state->setStorage($storage);
    }
    for ($i = 0; $i < $storage['number-of-rows']; $i++) {
      // Add 4 input elements to table row.
      $form['translations_table'][$i] = $this->buildFormTranslationRow($strings, $i);
    }

    $form['actions'] = [
      '#type' => 'actions',
    ];

    $form['actions']['add-row'] = [
      '#type' => 'submit',
      '#value' => $this->t('Add extra row'),
      '#submit' => ['::addExtraRow'],
      '#ajax' => [
        'callback' => '::addExtraRowAjaxCallback',
        'wrapper' => 'stringoverrides-wrapper',
      ],
    ];

    $form['actions']['remove'] = [
      '#type' => 'submit',
      '#value' => $this->t('Remove disabled strings'),
    ];

    $form['actions']['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Save configuration'),
    ];
    return $form;
  }

  /**
   * Get current translations in format for admin form.
   *
   * @param string $language
   *   Language code for configuration.
   *
   * @return array
   *   Translations.
   */
  public function getCurrentTranslations($language) {
    $config_factory = \Drupal::service('config.factory');
    $words_enabled = $config_factory
      ->getEditable('stringoverrides.string_override.' . $language)
      ->get('contexts');
    $words_disabled = $config_factory
      ->getEditable('stringoverrides.string_override.' . $language . '_disabled')
      ->get('contexts');

    $words = [
      FALSE => $words_disabled ? $words_disabled : [],
      TRUE => $words_enabled ? $words_enabled : [],
    ];

    $strings = [];
    foreach ($words as $enabled => $custom_strings) {
      foreach ($custom_strings as $context) {
        foreach ($context['translations'] as $source => $translation) {
          $strings[] = [
            'enabled' => $enabled,
            'context' => $context['context'],
            'source' => $translation['source'],
            'translation' => $translation['translation'],
          ];
        }
      }
    }
    // Sort alphabetically.
    usort($strings, function ($word1, $word2) {
      return strcasecmp($word1['source'], $word2['source']);
    });

    return $strings;
  }

  /**
   * Simplify buildForm function.
   *
   * @param array $strings
   *   Data for all translations.
   * @param int $row_no
   *   Row number.
   *
   * @return array
   *   One row for form table.
   */
  private function buildFormTranslationRow(array $strings, $row_no) {
    if (!empty($strings[$row_no])) {
      $string = $strings[$row_no];
    }
    else {
      $string = [
        'enabled' => TRUE,
        'source' => '',
        'translation' => '',
        'context' => '',
      ];
    }
    $row = [];
    $row['enabled'] = [
      '#type' => 'checkbox',
      '#maxlength' => 255,
      '#default_value' => $string['enabled'],
      '#attributes' => array(
        'title' => t('Flag whether this override should be active.'),
      ),
    ];

    $row['source'] = [
      '#type' => 'textarea',
      '#default_value' => $string['source'],
      '#rows' => 1,
      '#attributes' => array(
        'title' => t('The original source text to be replaced.'),
      ),
    ];

    $row['translation'] = [
      '#type' => 'textarea',
      '#default_value' => $string['translation'],
      '#rows' => 1,
      '#attributes' => array(
        'title' => t('The text to replace the original source text.'),
      ),
      // Hide the translation when the source is empty.
      '#states' => [
        'invisible' => [
          "#edit-translations-table-$row_no-source" => ['empty' => TRUE],
        ],
      ],
    ];

    $row['context'] = [
      '#type' => 'textfield',
      '#maxlength' => 255,
      '#default_value' => $string['context'],
      '#attributes' => [
        'title' => t('Strings sometimes can have context applied to them. Most cases, this is not the case.'),
      ],
      '#size' => 5,
      // Hide the context when the source is empty.
      '#states' => [
        'invisible' => [
          "#edit-translations-table-$row_no-source" => ['empty' => TRUE],
        ],
      ],
    ];
    return $row;
  }

  /**
   * Submit handler to add extra row.
   *
   * @param array $form
   *   Drupal form array.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   Drupal form state object.
   */
  public function addExtraRow(array $form, FormStateInterface $form_state) {
    $storage = $form_state->getStorage();
    $storage['number-of-rows']++;
    $form_state->setStorage($storage);
    $form_state->setRebuild(TRUE);
  }

  /**
   * Callback for Ajax functionality.
   *
   * @param array $form
   *   Drupal form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   Drupal form state object.
   *
   * @return mixed
   *   Form element for ajax response, it will replace table in browser.
   */
  public function addExtraRowAjaxCallback(array $form, FormStateInterface $form_state) {
    return $form['translations_table'];
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $language = $form_state->getValue('lang');
    // Format the words correctly for easy use then translating.
    $words = [TRUE => [], FALSE => []];
    // Drupal config key can't have some characters, we need index for each
    // contexts string and array to keep track .
    $config_enabled = \Drupal::service('config.factory')->getEditable('stringoverrides.string_override.' . $language);
    $config_disabled = \Drupal::service('config.factory')->getEditable('stringoverrides.string_override.' . $language . '_disabled');

    $form_data = $form_state->getValue('translations_table');
    foreach ($form_data as $i => $string) {
      if (!empty($string['source'])) {
        $context = $string['context'];
        list($source, $translation) = str_replace("\r", '', [$string['source'], $string['translation']]);
        $words[$string['enabled']][$context]['context'] = $context;
        $words[$string['enabled']][$context]['translations'][] = [
          'source' => $source,
          'translation' => $translation,
        ];
      }
    }
    ksort($words[TRUE]);
    ksort($words[FALSE]);

    // Convert string array key to numeric array key, because Drupal config
    // doesn't support some characters in config keys, and sort by context.
    $words[TRUE] = array_values($words[TRUE]);
    $words[FALSE] = array_values($words[FALSE]);
    $config_enabled->set('contexts', $words[TRUE]);
    $config_enabled->save();

    switch ($form_state->getTriggeringElement()['#id']) {
      case 'edit-submit':
        $config_disabled->set('contexts', $words[FALSE]);
        $config_disabled->save();
        drupal_set_message(t('Your changes have been saved.'));
        break;

      case 'edit-remove':
        $config_disabled->delete();
        drupal_set_message(t('The disabled strings have been removed.'));
        break;
    }

    // Delete cache for active translation of this language.
    \Drupal::cache()->delete('stringoverides:translation_for_' . $language);
  }

}
