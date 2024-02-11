<?php
/**
 * @file
 * Default Rules configurations.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function uc_affiliate2_default_rules_configuration() {
    // Setup a default configuration for customer checkout notifications.
  $rule = rules_reaction_rule();
  $rule->label = t('E-mail affiliate on new commission');
  $rule->active = TRUE;
  $rule->event('uc_affiliate2_apply_commission')
    ->condition('uc_affiliate2_condition_commission_amount', array(
      'commission:select' => 'uc_affiliate2_commission:commission',
      'operator' => 'greater',
      'amount' => '0',
    ))
    ->action('mail', array(
      'to:select' => 'uc_affiliate2_commission:affiliate:mail',
      'from' => uc_store_email_from(),
      'subject' => uc_get_message('uc_affiliate2_commission_subject'),
      'message' => uc_get_message('uc_affiliate2_commission_message'),
    ));

  $configs['uc_affiliate2_send_email'] = $rule;

  return $configs;
}
