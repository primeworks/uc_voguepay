<?php

/**
 * @file
 * Callback functions to handle responses from VoguePay.
 */


/**
*Helper code for VoguePay
*
*/

function uc_voguepay_checkout_review_form_submit() {
  // Invoke hook_order($op = 'submit') to test to make sure the order can
  // be completed... used for auto payment in uc_credit.module.
  $order = uc_order_load($_SESSION['cart_order']);
 // $error = FALSE;
  if(isset($_POST['transaction_id'])) {
//    dsm('It is here');
//    $order = uc_order_load($_SESSION['cart_order']);
    $_SESSION['do_complete'] = TRUE;
//    $form_state['redirect'] = 'cart/checkout/complete';
    drupal_goto('cart/checkout/complete');
  }
  else {
    drupal_goto('cart/checkout');
  }

}

