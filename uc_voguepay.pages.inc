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
    dsm('It is here');
//    $order = uc_order_load($_SESSION['cart_order']);
    $_SESSION['do_complete'] = TRUE;
//    $form_state['redirect'] = 'cart/checkout/complete';
    drupal_goto('cart/checkout/complete');
  }
  else {
    drupal_goto('cart/checkout');
  }
 //$_SESSION['cart_order'] == TRUE;
// if (isset($_SESSION['cart_order'])) {
 //  $order = variable_get('uc_voguepay_order', '');
 //  dsm($order->order_id);
 //  drupal_goto('cart/checkout/complete');
// }
// else {
//   drupal_goto('cart/checkout/review');
// }

}

function uc_vpay_complete($order) {
  // If the order ID specified in the return URL is not the same as the one in
  // the user's session, we need to assume this is either a spoof or that the
  // user tried to adjust the order on this side while at VoguePay. If it was a
  // legitimate checkout, the IPN will still come in from VoguePay so the order
  // gets processed correctly. We'll leave an ambiguous message just in case.
  if (intval($_SESSION['cart_order']) != $order->order_id) {
    drupal_set_message(t('Thank you for your order! VoguePay will notify us once your payment has been processed.'));
    drupal_goto('cart');
  }

  // Ensure the payment method is VoguePay WPS.
  if ($order->payment_method != 'voguepay_wps') {
    drupal_goto('cart');
  }

  // This lets us know it's a legitimate access of the complete page.
  $_SESSION['do_complete'] = TRUE;

  drupal_goto('cart/checkout/complete');
}
