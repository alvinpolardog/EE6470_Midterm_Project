
# EE6470 Midterm Project Report
吳哲廷 學號:110061590

Link to [Github Repo](https://github.com/alvinpolardog/EE6470_Midterm_Project)
##

## Implement and optimize an algorithm with HLS and ESL platform


For the Midterm Project, I chose to implement an algorithm that is essential in my lab work on brain activity. While NEO (Nonlinear Energy Operator) itself is a very simple process, it is very effective in what it propose to do, requires very little computational resources, and should be easily parallelizable. These are the reason why I chose NEO as my midterm project.

##  
## 

### Algorithm Introduction 

NEO or Nonlinear Energy Operator, refers to the following calculation:


![formula](https://i.imgur.com/469F8zA.png)

The main purpose of this calculation is to accentuate high-frequency content, and emphasize instantaneous changes. This is essential in detecting anomaly in EEG, where signals are often very tightly packed, making some spikes difficult to distinguish.

In this following example image, we can see a simulated signals containing multiple spikes. However it may be difficult to simply choose a threshold and find all the peaks, since the base of the signal is constantly fluctuating. In the NEO version, the signal becomes much more clear, and we can quickly differentiate between significant spikes and noise.


![NEO example image](https://i.imgur.com/s6QUo4z.png)

Using this formula as our kernel function, we can create a module that can process any single channel electroencephalogram easily and quickly, irregardless of length.

Another part of the spike detection process is deciding a robust threshold. One method is to choose a static amplitude, but this can often lead to false positive when the signal becomes overly noisy. Instead, I chose to implement an adaptive threshold that is based on rolling mean.

![](https://i.imgur.com/KMIqDYn.png)

##  
##  

### Design Experiments
#### Ver 1. NEO Module operating on one value at a time

![](https://i.imgur.com/WQdE5B1.png)

In the first version, the testbench read for INPUT.dat and send one value at a time to the NEO module. The NEO module saves the value in the internal memory buffer, and waits until at least three values are available. Once three vales are saved, the NEO loop begins, and write the result back to the testbench.

##### Optimization Simulation Results: 


| Optimization | Total Runtime (ns) | Time Difference | Area | Area Difference | Average Latency (Cycles) |
| -------- | -------- | -------- | ------ | ------ | ---- |
| No Optimization | 119950 | *benchmark*   | **2969.1** | *benchmark* | 5|
| DPOPT    | 119950     | ±0%     | 3358.0 | +13% | 5|
| LOOP_UNROLLING-4     | 134950     |+12%| 4360.5     | +46% | 5|
| LOOP_UNROLLING-8     | 139950     |+12%| 5311.5     | +78% | 5|
| PIPELINE-1     | **20040**     | **-83%**     | 4532.4| +52% | 3|
| PIPELINE-1 DPOPT   | **20040**     | **-83%**     | 4150.4 | +39% | 3|
| PIPELINE-2     | 40030     | -67%     | 3973.8 | +34% | 4|
| PIPELINE-2 DPOPT  | 40030     | -67%     | 4208.8 | +41% | 4|
#### Preliminary Conclusion

The first version works fairly well. Datapath optimzation did not work all the time, but performed well in the pipeline-1 case. Overall, while loop unrolling did not work well, perhaps due to poor constraint in timing, pipelining work extremely well, and lowered the simulation time by a factor of 5, although also increasing the area by a fair margin.

## 
##

#### Ver 2. NEO Module with large bandwidth


![](https://i.imgur.com/QtqDThb.png)

In the second version, the values are bundled as a single package in the testbench, and sent to the NEO module. In this version, the bundle size is set at 8. The NEO process is then split into three section, with the first two, "read to buffer" and  "solve for NEO" contain within its own loop. This allows us to specify the loop optimization more specifically, and perhaps increase throughput.

##### Optimization Simulation Results: 


| Optimization | Total Runtime (ns) | Time Difference | Area | Area Difference | Average Latency (Cycles) |
| -------- | -------- | -------- | ------ | ------ | ---- |
| No Optimization | 113730 | -5%   | 14542.7| +389% | 89|
| DPOPT    | 78050     | -35%     | 12515.8| +321% | 61|
| LOOP_UNROLLING-2     | 113730     |-5%| 21346.5    | +618% | 89|
| LOOP_UNROLLING-2 DPOPT     | 86970     |-38%| 17725.7     | +497% | 68|
| PIPELINE-1     |42370     | **-65%**     | 16040.4 | +440% | 32|
| PIPELINE-1 DPOPT   |42370     | **-65%**     | 9947.7 | +235% | 32|
| PIPELINE-2 DPOPT  | 60210     | -50%     | 11368.7 | +282% | 47|


#### Preliminary Conclusion

The second version performed poorly compared to the first version. In regard to simulated runtime, loop unrolling worked relatively well compared to before, but pipelining did not. This may be due to the fact that during HLS, the reading of the bundled values forced an addition wait() due to constraint and dependency issue, thus, making the runtime difference much less significant. 

In term of area, the second version performed terribly. The following two synthesis reports are for the two versions of pipeline-1.

VERSION_1 PIPELINE-1

               +--------------------------------------------------------------------------+
               |                                                                          |
        00803: | Allocation Report for all threads:                                       |
        00805: |                                             Area/Instance                |
        00805: |                                       -------------------------    Total |
        00805: |                      Resource  Count     Seq(#FF)    Comb    BB     Area |
        00805: | -----------------------------  -----  -----------  ------  ----  ------- |
        00807: |      NEO_input_buffer_regbank      1   827.6(110)   717.8         1545.5 |
        00807: | NEO_SubMul2s11s11Mul2s11s11_4      1               1401.9         1401.9 |
        00807: |                   mux_1bx2i1c     15                  2.7           40.7 |
        00807: |               NEO_Add2i1u11_4      1                 39.0           39.0 |
        00807: |                  mux_13bx2i1c      1                 35.3           35.3 |
        00807: |           NEO_N_Mux_11_2_21_4      1                 26.3           26.3 |
        00807: |                NEO_Add2i1u4_4      2                 10.6           21.2 |
        00807: |                 NEO_Subi1u4_4      2                  9.2           18.5 |
        00807: |                   mux_1bx2i2c      6                  2.3           14.0 |
        00807: |                NEO_LtiLLs13_4      1                 11.3           11.3 |
        00807: |            NEO_N_Mux_4_2_22_4      2                  5.5           10.9 |
        00807: |            NEO_N_Mux_4_2_20_4      2                  5.5           10.9 |
        00807: |                   mux_4bx2i1c      1                 10.9           10.9 |
        00807: |                   mux_1bx3i1c      2                  3.8            7.6 |
        00807: |            NEO_And_1Ux1U_1U_4      4                  1.4            5.5 |
        00807: |              NEO_gen_busy_r_4      1                  5.5            5.5 |
        00807: |                   mux_2bx2i1c      1                  5.4            5.4 |
        00807: |            NEO_Xor_1Ux1U_1U_1      1                  4.4            4.4 |
        00807: |          NEO_Equal_1Ux1U_1U_1      1                  4.4            4.4 |
        00807: |               NEO_Not_1U_1U_1      1                  4.1            4.1 |
        00807: |       NEO_OrReduction_4U_1U_4      2                  2.1            4.1 |
        00807: |                   mux_1bx3i2c      1                  3.5            3.5 |
        00807: |                 NEO_Eqi10u4_4      1                  2.7            2.7 |
        00807: |                  NEO_Eqi9u4_4      1                  2.7            2.7 |
        00807: |           NEO_N_Muxb_1_2_12_4      1                  2.4            2.4 |
        00807: |             NEO_Or_1Ux1U_1U_4      1                  1.4            1.4 |
        00807: |               NEO_Not_1U_1U_4      1                  0.7            0.7 |
        00808: |                     registers     31                                     |
        01442: |             Reg bits by type:                                            |
        01442. |                EN SS SC AS AC                                            |
        00809: |                 0  0  1  0  0      3     5.5(1)       1.4                |
        00809: |                 0  1  0  0  0      1     5.5(1)       1.4                |
        00809: |                 1  0  0  0  0     90     7.5(1)       0.0                |
        00809: |                 1  0  1  0  0      4     7.5(1)       1.4                |
        00809: |                 1  1  0  0  0      1     7.5(1)       1.4                |
        00809: |             all register bits     99     7.4(1)       0.1          749.0 |
        02604: |               estimated cntrl      1                160.6          160.6 |
        00811: | ------------------------------------------------------------------------ |
        00812: |                    Total Area         1564.3(209)  2586.1   0.0   4150.4 |
               |                                                                          |
               +--------------------------------------------------------------------------+
               
               
VERSION_2 PIPELINE-1

               +--------------------------------------------------------------------------+
               |                                                                          |
        00803: | Allocation Report for all threads:                                       |
        00805: |                                             Area/Instance                |
        00805: |                                       -------------------------    Total |
        00805: |                      Resource  Count     Seq(#FF)    Comb    BB     Area |
        00805: | -----------------------------  -----  -----------  ------  ----  ------- |
        00807: |      NEO_input_buffer_regbank      1   827.6(110)   717.3         1544.9 |
        00807: | NEO_SubMul2s11s11Mul2s11s11_4      1               1401.9         1401.9 |
        00807: |  NEO_RightShift_88Ux11S_11U_4      1                784.5          784.5 |
        00807: |  NEO_LeftShift_22Ux12S_176S_4      1                769.2          769.2 |
        00807: |      NEO_And_176Ux176U_176U_4      3                240.8          722.3 |
        00807: |        NEO_LeftShift_176U_3_4      1                451.1          451.1 |
        00807: |        NEO_LeftShift_176U_2_4      1                446.7          446.7 |
        00807: |       NEO_Or_176Sx176U_176S_4      1                240.8          240.8 |
        00807: |        NEO_NotBit_176U_176U_4      2                120.4          240.8 |
        00807: |        NEO_LeftShift_88U_32_4      1                221.3          221.3 |
        00807: |         NEO_And_88Sx88U_88U_4      1                120.4          120.4 |
        00807: |          NEO_Mul2i22Subi1u4_4      1                 65.3           65.3 |
        00807: |          NEO_Mul2i11Subi1u4_4      1                 65.3           65.3 |
        00807: |          NEO_NotBit_88U_88U_4      1                 60.2           60.2 |
        00807: |          NEO_Subi1Mul2i22u4_4      1                 50.6           50.6 |
        00807: |          NEO_Subi1Mul2i11u4_4      1                 50.6           50.6 |
        00807: |                   mux_4bx2i1c      4                 10.9           43.5 |
        00807: |                   mux_4bx2i0c      2                 12.4           24.8 |
        00807: |               NEO_Add_6S_15_4      2                 12.0           23.9 |
        00807: |                NEO_Add2i1u4_4      2                 10.6           21.2 |
        00807: |                 NEO_Subi1u4_4      2                  9.2           18.5 |
        00807: |            NEO_N_Mux_4_2_39_4      3                  5.5           16.4 |
        00807: |                   mux_3bx5i4c      1                 14.7           14.7 |
        00807: |                   mux_1bx2i2c      6                  2.3           14.0 |
        00807: |            NEO_N_Mux_4_2_38_4      2                  5.5           10.9 |
        00807: |                  NEO_Lei8s6_4      2                  4.8            9.6 |
        00807: |                 NEO_Eqi10u4_4      2                  2.7            5.5 |
        00807: |            NEO_Xor_1Ux1U_1U_1      1                  4.4            4.4 |
        00807: |               NEO_Not_1U_1U_1      1                  4.1            4.1 |
        00807: |              NEO_gen_busy_r_4      1                  4.1            4.1 |
        00807: |       NEO_OrReduction_4U_1U_4      2                  2.1            4.1 |
        00807: |                   mux_1bx3i1c      1                  3.8            3.8 |
        00807: |                  NEO_Eqi9u4_4      1                  2.7            2.7 |
        00807: |           NEO_N_Muxb_1_2_19_4      1                  2.4            2.4 |
        00807: |            NEO_And_1Ux1U_1U_4      1                  1.4            1.4 |
        00807: |             NEO_Or_1Ux1U_1U_4      1                  1.4            1.4 |
        00808: |                     registers     15                                     |
        01442: |             Reg bits by type:                                            |
        01442. |                EN SS SC AS AC                                            |
        00809: |                 0  0  1  0  0      2     5.5(1)       1.4                |
        00809: |                 0  1  0  0  0      1     5.5(1)       1.4                |
        00809: |                 1  0  0  0  0    101     7.5(1)       0.0                |
        00809: |                 1  0  1  0  0    183     7.5(1)       1.4                |
        00809: |                 1  1  0  0  0      1     7.5(1)       1.4                |
        00809: |             all register bits    288     7.5(1)       0.9         2416.6 |
        02604: |               estimated cntrl      1                 63.9           63.9 |
        00811: | ------------------------------------------------------------------------ |
        00812: |                    Total Area         2988.4(398)  6959.3   0.0   9947.7 |
               |                                                                          |
               +--------------------------------------------------------------------------+
               
At a quick glance, we can clearly see the difference lies in the number of register bits. The reason for this large discrepancy is most likely due to the large number of register required for reading from the input inteface to the internal buffer.

## 
##  

#### Ver 3. NEO Module with variable bandwidth


![](https://i.imgur.com/QtqDThb.png)

The third version of the NEO module follows mostly in the footstep of the second version. Here, we The code for the buffer-loading sequence was rewritten, thus eliminating the extra wait(), and the value bundling step was changed to allow different bandwidth using an annotation in 'define.h'. The optimization used in this version was mainly only pipelining, as it performed consistently superiorly in previous versions.


##### Optimization Simulation Results: 


| Optimization | Total Runtime (ns) | Time Difference | Area | Area Difference | Average Latency (Cycles) |
| -------- | -------- | -------- | ------ | ------ | ---- |
| BANDWIDTH = 2 (PIPELINE-1 DPOPT)  | 50060     | -58%      | **5305.8**| **+79%** | 8|
| BANDWIDTH = 3 (PIPELINE-1 DPOPT)  | 40020     | -76%     | 6263.4| +111% | 10|
| BANDWIDTH = 4 (PIPELINE-1 DPOPT)  | 35060     | -70%       | 7242.0    | +144% | 12|
| BANDWIDTH = 5 (PIPELINE-1 DPOPT)  | 32060     |-73%      | 8104.5     | +173% | 14|
| BANDWIDTH = 6 (PIPELINE-1 DPOPT)  | 30060     | -75%     | 8958.9 | +202% | 16|
| BANDWIDTH = 7 (PIPELINE-1 DPOPT)  | 28600     | -76%     | 9890.9 | +233% | 18|
| BANDWIDTH = 8 (PIPELINE-1 DPOPT)  | **27500**     | **-77%**     | 10781.5 | +263% | 20|



#### Preliminary Conclusion

The third version performed similarly to the second version, but by graphing the results, we can clearly see a pattern emerging.

![](https://i.imgur.com/HRHdV5l.png)

As we increase the input bandwidth, the simulated time decrease logarithmically, but the area consumed increase linearly. It is apparent that the runtime is reaching its lower limit with this architecture, and that increasing the bandwidth even further will not result in lower runtime.


#### Ver 4. NEO Module with pipelined unrolled loops

![](https://i.imgur.com/8OI9FWk.png)

After looking at all the versions, the first version using pipeline still had the best runtime, and so I begin comparing the difference, and tried to figure out why. In version 4, instead of separating each loop, the loops for each steps are unrolled, and allowed for the entire NEO procedure to run in a pipeline similar to the best performing optimization in version 1.


##### Optimization Simulation Results: 


| Optimization | Total Runtime (ns) | Time Difference | Area | Area Difference | Average Latency (Cycles) |
| -------- | -------- | -------- | ------ | ------ | ---- |
| BANDWIDTH = 2 (PIPELINE-1 & UNROLLED LOOPS)  | 10040      | -91%      | 7093.3| +139% | 3|
| BANDWIDTH = 3 (PIPELINE-1 & UNROLLED LOOPS)  | 6710    | -94%     | 10457.2| +252% | 3|
| BANDWIDTH = 4 (PIPELINE-1 & UNROLLED LOOPS)  | **5040**    | **-95%**     |  13554.7| +356% | 3|



#### Preliminary Conclusion
The forth version ran extremely fast, with a runtime reduction of upto 95%. By unrolling the inner loops, we allowed the multiple caluculation to be done in just 1 cycle, and the pipeline allowed the overall runtime to be much shorter.

VERSION 4  BANDWIDTH = 3 (PIPELINE-1 & UNROLLED LOOPS)

               +--------------------------------------------------------------------------+
               |                                                                          |
        00803: | Allocation Report for all threads:                                       |
        00805: |                                             Area/Instance                |
        00805: |                                       -------------------------    Total |
        00805: |                      Resource  Count     Seq(#FF)    Comb    BB     Area |
        00805: | -----------------------------  -----  -----------  ------  ----  ------- |
        00807: |         NEO_Mul_11Sx11S_22S_4      4                736.7         2946.7 |
        00807: | NEO_SubMul2s11s11Mul2s11s11_4      1               1401.9         1401.9 |
        00807: |          NEO_N_Mux_11_10_32_4      6                170.7         1023.9 |
        00807: |           NEO_N_Mux_11_2_31_4     30                 26.3          790.0 |
        00807: |          NEO_N_Mux_11_10_33_4      3                174.1          522.2 |
        00807: |         NEO_Sub_22Ux22U_22U_4      2                196.7          393.3 |
        00807: |                  mux_11bx2i1c      8                 29.9          239.0 |
        00807: |          NEO_N_MuxB_33_2_28_4      1                 79.0           79.0 |
        00807: |                  mux_11bx2i0c      2                 34.1           68.3 |
        00807: |                NEO_Add2i1u4_4      6                 10.6           63.6 |
        00807: |            NEO_N_Mux_4_2_30_4      8                  5.5           43.8 |
        00807: |                   mux_1bx2i1c     10                  2.7           27.2 |
        00807: |                  NEO_Eqi9u4_4      6                  2.7           16.4 |
        00807: |                 NEO_Eqi10u4_4      5                  2.7           13.7 |
        00807: |                   mux_4bx2i1c      1                 10.9           10.9 |
        00807: |                   mux_1bx2i2c      4                  2.3            9.3 |
        00807: |                  NEO_Eqi7u4_4      3                  3.1            9.2 |
        00807: |                 NEO_Subi1u4_4      1                  9.2            9.2 |
        00807: |                  NEO_Eqi3u4_4      3                  2.7            8.2 |
        00807: |                  NEO_Eqi5u4_4      3                  2.7            8.2 |
        00807: |                  NEO_Eqi6u4_4      3                  2.7            8.2 |
        00807: |                  NEO_Eqi1u4_4      3                  2.4            7.2 |
        00807: |                  NEO_Eqi2u4_4      3                  2.4            7.2 |
        00807: |                  NEO_Eqi4u4_4      3                  2.4            7.2 |
        00807: |                  NEO_Eqi8u4_4      3                  2.4            7.2 |
        00807: |       NEO_OrReduction_4U_1U_4      3                  2.1            6.2 |
        00807: |            NEO_And_1Ux1U_1U_4      4                  1.4            5.5 |
        00807: |              NEO_gen_busy_r_4      1                  5.5            5.5 |
        00807: |            NEO_N_Mux_4_2_29_4      1                  5.5            5.5 |
        00807: |                   mux_2bx2i1c      1                  5.4            5.4 |
        00807: |            NEO_Xor_1Ux1U_1U_1      1                  4.4            4.4 |
        00807: |               NEO_Not_1U_1U_1      1                  4.1            4.1 |
        00807: |                   mux_1bx3i1c      1                  3.8            3.8 |
        00807: |                   mux_1bx3i2c      1                  3.5            3.5 |
        00807: |           NEO_N_Muxb_1_2_19_4      1                  2.4            2.4 |
        00807: |             NEO_Or_1Ux1U_1U_4      1                  1.4            1.4 |
        00807: |               NEO_Not_1U_1U_4      1                  0.7            0.7 |
        00808: |                     registers     64                                     |
        01442: |             Reg bits by type:                                            |
        01442. |                EN SS SC AS AC                                            |
        00809: |                 0  0  1  0  0      3     5.5(1)       1.4                |
        00809: |                 0  1  0  0  0      1     5.5(1)       1.4                |
        00809: |                 1  0  0  0  0    339     7.5(1)       0.0                |
        00809: |                 1  0  1  0  0      4     7.5(1)       1.4                |
        00809: |                 1  1  0  0  0      1     7.5(1)       1.4                |
        00809: |             all register bits    348     7.5(1)       0.0         2622.5 |
        02604: |               estimated cntrl      1                 65.5           65.5 |
        00811: | ------------------------------------------------------------------------ |
        00812: |                    Total Area         2610.1(348)  7847.1   0.0  10457.2 |
               |                                                                          |
               +--------------------------------------------------------------------------+
               
Looking at the synthesis result of version 4, we can see that most of the area is taken up by large multiplier and arithmetic resouces as well as a large number of multiplexer. Since almost all of the calculation for the kernel function is done in parallel, the large amount of resource required is expected.

## Final Conlusion

Take a look at the full comparison.

| Optimization | Total Runtime (ns) | Time Difference | Area | Area Difference | Average Latency (Cycles) |
| -------- | -------- | -------- | ------ | ------ | ---- |
| [V1] No Optimization | 119950 | *benchmark*   | **2969.1** | *benchmark* | 5|
| [V1] DPOPT    | 119950     | ±0%     | 3358.0 | +13% | 5|
| [V1] LOOP_UNROLLING-4     | 134950     |+12%| 4360.5     | +46% | 5|
| [V1] LOOP_UNROLLING-8     | 139950     |+12%| 5311.5     | +78% | 5|
| [V1] PIPELINE-1     | 20040     | -83%     | 4532.4| +52% | 3|
| [V1] PIPELINE-1 DPOPT   | 20040     | -83%     | 4150.4 | +39% | 3|
| [V1] PIPELINE-2     | 40030     | -67%     | 3973.8 | +34% | 4|
| [V1] PIPELINE-2 DPOPT  | 40030     | -67%     | 4208.8 | +41% | 4|
| [V2] No Optimization | 113730 | -5%   | 14542.7| +389% | 89|
| [V2] DPOPT    | 78050     | -35%     | 12515.8| +321% | 61|
| [V2] LOOP_UNROLLING-2     | 113730     |-5%| 21346.5    | +618% | 89|
| [V2] LOOP_UNROLLING-2 DPOPT     | 86970     |-38%| 17725.7     | +497% | 68|
| [V2] PIPELINE-1     |42370     | -65%     | 16040.4 | +440% | 32|
| [V2] PIPELINE-1 DPOPT   |42370     | -65%     | 9947.7 | +235% | 32|
| [V2] PIPELINE-2 DPOPT  | 60210     | -50%     | 11368.7 | +282% | 47|
| [V3] BANDWIDTH = 2 (PIPELINE-1 DPOPT)  | 50060     | -58%      | 5305.8| +79% | 8|
| [V3] BANDWIDTH = 3 (PIPELINE-1 DPOPT)  | 40020     | -76%     | 6263.4| +111% | 10|
| [V3] BANDWIDTH = 4 (PIPELINE-1 DPOPT)  | 35060     | -70%       | 7242.0    | +144% | 12|
| [V3] BANDWIDTH = 5 (PIPELINE-1 DPOPT)  | 32060     |-73%      | 8104.5     | +173% | 14|
| [V3] BANDWIDTH = 6 (PIPELINE-1 DPOPT)  | 30060     | -75%     | 8958.9 | +202% | 16|
| [V3] BANDWIDTH = 7 (PIPELINE-1 DPOPT)  | 28600     | -76%     | 9890.9 | +233% | 18|
| [V3] BANDWIDTH = 8 (PIPELINE-1 DPOPT)  | 27500     | -77%     | 10781.5 | +263% | 20|
| [V4] BANDWIDTH = 2 (PIPELINE-1 & UNROLLED LOOPS)  | 10040      | -91%      | 7093.3| +139% | 3|
| [V4] BANDWIDTH = 3 (PIPELINE-1 & UNROLLED LOOPS)  | 6710    | -94%     | 10457.2| +252% | 3|
| [V4] BANDWIDTH = 4 (PIPELINE-1 & UNROLLED LOOPS)  | **5040**    | **-95%**     |  13554.7| +356% | 3

At the two extremes, if we want only the lowest area, the original version without optimization is the smallest. If we only care about speed, the final version with pipelined unrolled loops run the fastest, albeit with much greater area.

If we want something down the middle, pipelined versions with fewer inputs work quite well, and does not use too much area.

### Application
Since I had some actual patient data on hand, I snipped out 2000 samples, and plot out the results of the module.

![](https://i.imgur.com/nqWmbt0.png)

The blue line indicate the original EEG signal, and the orange line is the output of the NEO module. We can see that the spikes in the blue line is somewhat difficult to makeout, especially if we had to look at multiple channels at the same time, or look at longer period of data. The spike on the orange line however is very apparent and distinguishing the spike should be much easier.

![](https://i.imgur.com/0EijQA5.png)

Here, we added an additional adaptive threshold calculated using pure software. It is clear that the threshold will easily weed out any unwanted noise, allowing us to quickly discern where the spikes are. Currently, the threshold caluculation is not done on the NEO module since it uses requires a buffer size of at least a few hundred samples to work properly, and thus would completely overwhelm the area size, making our comparisons between different architecture meaningless. 

However just with the NEO, we can see that using HLS, we can produce a design that is practical l in the real-time processing of EEG, and that the optimization directives allow us to speed up the design by a factor of nearly 20.