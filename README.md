# IEEE 802.15.4 Multi-PHY TSCH with sub-GHz support

This repository contains the source code of TSCH for 868 MHz and the multi-band support for TSCH. To learn more about it , check out our IEEE LCN'19 paper -- *IEEE 802.15.4 TSCH in Sub-GHz: Design Considerations and Multi-band Support*.

## Code Layout

*Disclaimer: Although we tested the code extensively, it is a research prototype that likely contains bugs. We take no responsibility for and give no warranties in respect of using the code.*



The multi-PHY used to characterize the wireless medium using the single-template design can be found at:

```
/projects/tsch-multiPHY
```



The multi-PHY with TSCH beacons at 1.2 kbps and IPv6 traffic at 1000 kbps using the multi-template design can be found at:

```bash
/project/tsch-rpl-multiPHY
```
