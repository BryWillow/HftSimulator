#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>
#include "spsc_ringbuffer.h"
#include "itch_connection.h"
#include "itch_message.h"
#include "itch_processor.h"
#include "itch_sender.h"

int main()
{
    // Market Data:
    // CME / BrokerTec: CME FIX/SBE (MDP 3.0) -- much faster to parse
    // eSpeed: NSDQ ITCH-style Binary
    // Both are multicast feeds with TCP recovery.

    //ItchProcessor itchProcessor;
    ItchSender itchSender;

    SpScRingBuffer<ItchMessage, 2> ringBuffer;

    for(int j = 0; j < 4; j++) {
        ItchMessage i;
        i.msgType = 'A';
        i.orderId = '1';
        i.price = 0;
        i.quantity = 3;
        i.side = 'B';
        strcpy(i.symbol, "ASDF");

        ringBuffer.tryPush(i);        
    }

    std::cout << "Br" << std::endl;
    // Order Entry:
    // CME / BrokerTec: version of FIX
    // eSpeed: version of FIX

    // Memory Orderings
}