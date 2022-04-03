#pragma version 1.0
#program name SignumBillboard1
#program description SignumBillboard.io smart contract #1
#include APIFunctions

// Minimum segment cost
#define ONE_SIGNA 1_0000_0000
#program activationAmount ONE_SIGNA
// Rounded execution reserve to reject insufficient bids
#define EXECUTION_RESERVE_AMOUNT 2000_0000
// Price is halved every 7 days
#define BLOCKS_TO_HALVE_THE_PRICE 2520
// Signum-Network Association address
#define SNA_ADDRESS "S-5MS6-5FBY-74H4-9N4HS"

// Transaction structure
struct TXINFO
{
    long amount;
    long tax;
    long timestamp;
    long sender;
} currentTX, ownerTX;

// Program main body
void main(void) {
    // read all income transactions
    for (A_To_Tx_After_Timestamp(currentTX.timestamp); Get_A1() != 0; A_To_Tx_After_Timestamp(currentTX.timestamp)) {
        // save to currentTX temporary variable
        getTxDetails();
        // process it
        processTX();
    }
    // send remainder balance to creator
    sendCreatorTax();
}

// Fill currentTX temporary variable
void getTxDetails(void) {
    currentTX.amount = Get_Amount_For_Tx_In_A() + ONE_SIGNA;
    // tax will be doubled: 1% to SNA + 1% to creator
    currentTX.tax = currentTX.amount / 100;
    currentTX.timestamp = Get_Timestamp_For_Tx_In_A();
    B_To_Address_Of_Tx_In_A();
    currentTX.sender = Get_B1();
}

// Process transaction
void processTX(void) {
    // calc actual price
    long blocksDiff = (currentTX.timestamp - ownerTX.timestamp) >> 32;
    long actualPrice = (ownerTX.amount * 2) >> (blocksDiff / BLOCKS_TO_HALVE_THE_PRICE);
    if (actualPrice < ONE_SIGNA) {
        actualPrice = ONE_SIGNA;
    }
    // if current amount outbid the actual price
    if (currentTX.amount >= actualPrice) {
        // return previous amount to it owner
        if (ownerTX.sender != 0) { // if not first purchase
            Set_B1(ownerTX.sender);
            Send_To_Address_In_B(ownerTX.amount - ownerTX.tax * 2  - EXECUTION_RESERVE_AMOUNT);
        }

        // switch the owner
        ownerTX.amount = currentTX.amount;
        ownerTX.tax = currentTX.tax;
        ownerTX.timestamp = currentTX.timestamp;
        ownerTX.sender = currentTX.sender;

        // and donate one tax to SNA
        sendDonateToSNA();
    } else {
        // negative answer if not enough to own
        Set_B1(currentTX.sender);
        Set_A1_A2("Not enou", "gh bid!"); Set_A3_A4("", "");
        Send_A_To_Address_In_B();
        Send_To_Address_In_B(currentTX.amount - EXECUTION_RESERVE_AMOUNT);
    }
}

// Donate tax to SNA
void sendDonateToSNA(void) {
    Set_B1(SNA_ADDRESS);
    Send_To_Address_In_B(ownerTX.tax);
}

// Send remainder balance to creator
void sendCreatorTax(void) {
    if (ownerTX.amount > 0) {
        B_To_Address_Of_Creator();
        // as can't send tax to the creator every time
        // coz don't know if the purchase was successful or not
        // leave only owner balance minus SNA tax (it's already sent)
        Send_To_Address_In_B(Get_Current_Balance() + ownerTX.tax - ownerTX.amount);
    }
}
