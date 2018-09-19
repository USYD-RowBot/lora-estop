# lora-estop
This is the codebase for the wireless emergency stops developed for the USYD RowBot's 2018 WAM-V AMS.

Functional Description:
The handheld unit acts to quickly echo all signals sent by the On-Vehicle Controller (OVC). The emergency stop button cuts power to this unit when pressed so it can't receive or transmit anything.

The OVC maintains power to an IIC relay unit, with the E-Stop control line running through its NO connector. If the OVC ever loses power, the relay will default to open and the system E-Stop will trigger.
When operating, the OVC runs a watchdog timer which will de-energise the relay if it ever times out. Whenever the timer is reset, the OVC transmits a pseudo-random 8-bit number to the handheld unit. If a return message with matching number is received, the watchdog timer is reset and the process restarts. If no return message is received before the timer expires, a stop signal is sent to the relay, de-energising it and triggering the system E-Stop.
Even in the stopped state, the timer/tx/rx cycle continues. If a valid return is received, the OVC state reverts to normal and the relay is re-energised to return the system to normal operation. This return can only occur if the handheld unit is on, which can only occur if the handheld unit's E-Stop button is released. This indicates a "return to operation" command which the OVC follows.



The Responsibility Bit:

This code has been designed to be simple and effective, and I've spent time to make it as reliable as I reasonably can.
Understand that our best efforts to make something "safe" don't mean that this is a certified safety device.
If you make something like this, put it on a vehicle and someone gets hurt because it doesn't do what you expected, don't come crying to us. We use this because we understand the risks involved and we've taken steps beyond just this code to mitigate them to our own satisfaction.
