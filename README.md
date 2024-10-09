[![The link to the paper]](http://www.es.mdu.se/publications/6988-Forte__Hybrid_Traffic_Aware_Scheduling_for_Mobile_TSCH_Nodes)


# Forte: Mobility Management for TSCH Networks

This is an extension of the Contiki-NG project. 

Run the example/6tisch/new-schedule using the csc files such as 1mobility.csc in cooja.





Applications of the Internet of Things (IoT), particularly within Industrial IoT, impose stricter reliability and efficiency requirements on low-power wireless technologies. This has driven the creation of new medium access protocols, such as Time Slotted Channel Hopping (TSCH). Recently, autonomous schedulers, which manage wireless links without node negotiation, are gaining popularity due to their lightweight and reliable operation. However, challenges arise with node mobility and dynamic traffic, as current schedulers use a static allocation method. To overcome this gap, we propose Forte, a hybrid scheduler that combines autonomous scheduling for basic connectivity with a centralized on-demand scheduler that allocates extra timeslots and frequency channels so that nodes adapt to the dynamic requirements. The centralized module formulates a Lyapunov optimization to guarantee queue stability while minimizing negotiation overhead and nodes' duty-cycles. Forte outperforms the state-of-the-art by reducing packet end-to-end delay and increasing packet delivery ratio, all with minimal duty-cycle increase.


Bibtex
@inproceedings{Rabet6988,
author = {Iliar Rabet and Hossein Fotouhi and M{\'a}rio Alves and Maryam Vahabi and Mats Bj{\"o}rkman},
title = {Forte: Hybrid Traffic-Aware Scheduling for Mobile TSCH Nodes},
month = {October},
year = {2024},
booktitle = {IEEE Conference on Local Computer Networks },
url = {http://www.es.mdu.se/publications/6988-}
}
