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
  /*
 * Verify if the user has actually carried out a payment on VoguePay and is on the transaction complete page
 */

//Get current path of page
$path = drupal_get_path_alias($_GET['q']);
global $user;
//If there was a transaction_id sent from VoguePay send the dsm to the page
if (isset($_POST['transaction_id'])) {
//    dsm ('Score');
    //Check if the transaction is a valid one from VoguePay
    //get the full transaction details as an json from voguepay
    $json = file_get_contents('https://voguepay.com/?v_transaction_id='.$_POST['transaction_id'].'&type=json');
    //create new array to store our transaction detail
    $transaction = json_decode($json, true);

    /*
    Now we have the following keys in our $transaction array
    $transaction['merchant_id'],
    $transaction['transaction_id'],
    $transaction['email'],
    $transaction['total'],
    $transaction['merchant_ref'],
    $transaction['memo'],
    $transaction['status'],
    $transaction['date'],
    $transaction['referrer'],
    $transaction['method']
    */

    if($transaction['total'] == 0)die('Invalid total');
    if($transaction['status'] != 'Approved')die('Failed transaction');

    /*You can do anything you want now with the transaction details or the merchant reference.
    You should query your database with the merchant reference and fetch the records you saved for this transaction.
    Then you should compare the $transaction['total'] with the total from your database.*/
 // $error = FALSE;
  if(isset($_POST['transaction_id'])) {
//    dsm('It is here');
//    $order = uc_order_load($_SESSION['cart_order']);
    $_SESSION['do_complete'] = TRUE;
    global $user;
 //   db_query("UPDATE {uc_orders} SET order_status = 'payment_received' WHERE order_id = %d  AND payment_method = 'voguepay'", $order->order_id);
//    $form_state['redirect'] = 'cart/checkout/complete';
    drupal_goto('cart/checkout/complete');
    dsm($order->order_id);
  }
  else {
    drupal_goto('cart/checkout');
  }

}
}
