## MCP3208read
MCP3208 AD converter driver for Raspberry pi

### MCP3208 をハンドリングしてデータを取り込みます。
  
  戻り値はhexadecimal値になります。'0xFFF'のような感じ。

```  
  NAME:
     MCP3208read
  
  SYNOPSIS:
     MCP3208read [-Dsdmc]
  
  OPTIONS:
	[-D | --device] device_file	: Specify the device file. /dev/spidev0.0 is the default of this option.
	[-c | --channel] channel	: Select the input channel of AD converter. '1' as the default of this option. See    MCP3208 data sheet
	[-s | --speed ] frequency	: apply clock speed in Hz. 500kHz: Default
	[-d | --delay ] time usec	: apply delay in usec. 0us: Default
	[-m | --mode  ] SE/nDIFF	: Set measurement mode [Single-end(1)/Diff(0)] Diff mode: Default
  
  NEEDS: SPI interface 
```
