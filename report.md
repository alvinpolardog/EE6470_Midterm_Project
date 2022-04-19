
# EE6470 Midterm Project Report
吳哲廷 學號:110061590

Link to [Github Repo](https://github.com/alvinpolardog/EE6470_Midterm_Project)
##

## Implement and optimize an algorithm with HLS and ESL platform


For the Midterm Project, I chose to implement an algorithm that is essential in my lab work. While NEO (Nonlinear Energy Operator) itself is a very simple process, it is very effective in what it propose to do, and require very little computational resources. That is the reason why I chose NEO as my midterm project.

##  
## 

### Algorithm Introduction 

NEO or Nonlinear Energy Operator, refers to the following calculation 
<p align="center">
<img src="https://latex.codecogs.com/svg.image?\inline&space;\large&space;\bg{white}\Psi&space;(n)&space;=&space;x^2&space;(n)&space;-&space;x(n&plus;1)&space;*&space;x(n-1)" />
</p>


![](https://i.imgur.com/s6QUo4z.png)



##  
##  

#### GaussFilter implementation
HW2 uses the same Gaussian filter implemented in HW1. The Gaussian blur used in this implementation is a 3x3 mask:
 ```
    1, 2, 1
    2, 4, 2
    1, 2, 1
```
and a multiplier factor of 1/16 to avoid brightness oversaturation.

The row buffer used in HW1 is also implemented in this new TLM version, with the same three-row array that is constantly overwritten to store 
the pixels currently being used.

Contrary to HW1 however, my implementation discarded the triple channels for returning each color of the results. Since we are using a 4-byte pointer
for transfering information between the testbench and GaussFilter, after calculating a full row of pixel similar to in HW1, the results are converted 
into a 4 byte integer and travels through the o_result FIFO channel before being sent back to the testbench again through the data_ptr.

##  
## 

#### Overall Architecture
The overall connection in HW2 is identical to Lab4, albeit with the information being transfered more similar to that of HW1.

The testbench sends a single row at a time through its initiator sockets, with the destination address set to the GaussFilter module. The SimpleBus intercepts
the transaction, route it toward the GaussFilter, and map the address into the local offset. The local offset is then used to provide the correct context
for the information that is to be send back to the testbench, such as either the output result or the number of results available in the buffer.

After receiving all the rows of the target image back from GaussFilter, the entire bitmap along with the header is then sent to the virtual memory through two
individual transactions. Finally the memory is dumped to out.bmp.

##  
## 

#### Final Discussion
The change from just FIFO channels to TLM for the transaction between modules allow much better intercompatibility, this is evident in the fact that all the
transaction can be routed through the SimpleBus module, and that any changes made in one module may no longer require changes in another. By following the 
standard set out in TLM2.0, we will be able to incorporate other TLM modules in our designs without worry for incompatibility.

##  
## 

