# README

Test program to write test programs


## SWO ITM0 traces
To use swo tyou have to unsolder the jumper R93 and add the junmper R92.
Then run the following command. Adjust 168000000 to the cpu frequency.
```bash
openocd -f interface/stlink-v2-1.cfg \
		-c "transport select hla_swd" \
		-f target/stm32f7x.cfg \
		-c "tpiu config internal swo-itm0.log uart off 168000000 2000000"
```
to display the swo trace use tail or so
```bash
tail -f swo-itm0.log
```
do not forget to remove the log once you're finished.
