import os
import glob
import time
import datetime
import pandas as pd
import subprocess
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt
from pylab import *


dirpath = os.getcwd()

report = []



# Python code to sort the tuples using second element  
# of sublist Inplace way to sort using sort() 
def Sort(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using first element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: x[0]) 
    return sub_li 

#Open Results File
print('Search for *.csv Files...')
extension = 'csv'
result = glob.glob('result_files\*.{}'.format(extension))
ind = 0
for res in result:
    print(str(ind) + " for File: " + str(res[13:]))
    ind+= 1

x = input("Please type Number followed by enter to choose a file as Results file: ")
print(str(result[int(x)]) + " Will be loaded...")

#path = dirpath + "\\Analysis_Data_" + str(result[int(x)])
path = dirpath + "\\result_files\Analysis_Data_Test.csv"

df = pd.read_csv(result[int(x)],sep=';',encoding='utf-8')


def clean_and_calc_avg(ls):
    #Clean Latency, Throughput and PacketLoss Data and Calculate Avg
    last_val = 0
    ind = 0
    avg = 0
    cnt = 0
    while ind < len(ls):
        if ls[ind] == '-':
            ls[ind] = last_val
        else:
            last_val = ls[ind]
            avg += ls[ind]
            cnt += 1
        ind += 1
    return avg / cnt
    
    

def doAnalysis(df):
    df.sort_values(by=['Timestamp (us)'], inplace=True) #Sort by Timestamp
    df = df.reset_index(drop=True) #Recreate Index
    #Delete Radio Times
    for ind in df.index:
        if str(df['Source Address'][ind]) == "0":
            df.drop(ind,inplace=True)
        else:
            break
    df = df.reset_index(drop=True) #Recreate Index
    #Timestamp Nulling
    offset_us = df['Timestamp (us)'][0]
    for ind in df.index:
        df['Timestamp (us)'][ind] = df['Timestamp (us)'][ind] - offset_us
    
    #Latency LIst init
    latency = ['-' for i in range(len(df.index))]
    latency_per_hop = ['-' for i in range(len(df.index))]
    # latency_per_hop_per_server_cdc_y = defaultdict(list)
    # latency_per_hop_per_server_cdc_x = defaultdict(list)
    # latency_cdc = ['-' for i in range(len(df.index))]
    # latency_per_hop_cdc = ['-' for i in range(len(df.index))]
    throughput = ['-' for i in range(len(df.index))]
    pktloss = ['-' for i in range(len(df.index))]
    server_cnt = ['-' for i in range(len(df.index))]
    srv_addr_seen = []
    
    #Get Client Message
    for ind_cli in df.query('RSSI == "0"').index:
        #Get corresponding Server Message
        group_servers_cnt = 0
        pkt_arrived_cnt = 0
        srv_addr_seen.clear()
        for ind_srv in df.query('RSSI != "0"').index:
            if df['Message ID'][ind_cli] == df['Message ID'][ind_srv]:
                latency[ind_srv] = (df['Timestamp (us)'][ind_srv] - df['Timestamp (us)'][ind_cli]) / 1000
                latency_per_hop[ind_srv] = (df['Timestamp (us)'][ind_srv] - df['Timestamp (us)'][ind_cli]) / 1000 / (df['Hops'][ind_srv] + 1)
                #latency_per_hop_per_server_cdc_y[df['Destination Address'][ind_srv]].append(latency_per_hop[ind_srv])
                #latency_per_hop_per_server_cdc_x[df['Destination Address'][ind_srv]].append(df['Timestamp (us)'][ind_srv])
                if latency[ind_srv] > 0:
                    throughput[ind_srv] = df['Data Size'][ind_srv] / (latency[ind_srv] / 1000)            
            #Get the Group Matches of servers
            if df['Group Address'][ind_cli] == df['Group Address'][ind_srv]:
                if df['Message ID'][ind_cli] == df['Message ID'][ind_srv]:
                    pkt_arrived_cnt +=1
                if not df['Destination Address'][ind_srv] in srv_addr_seen:
                    group_servers_cnt +=1
                    srv_addr_seen.append(df['Destination Address'][ind_srv])
        server_cnt[ind_cli] = group_servers_cnt
        if group_servers_cnt == 0:
            pktloss[ind_cli] = 100 # 100% Packet Loss
        else:
            pktloss[ind_cli] = (group_servers_cnt - pkt_arrived_cnt) / group_servers_cnt * 100

    # #Correct CdC
    # for srv in latency_per_hop_per_server_cdc_y:
    #     x = latency_per_hop_per_server_cdc_x[srv]
    #     y_diffs = pd.DataFrame(latency_per_hop_per_server_cdc_y[srv]).diff().values.tolist()
    #     prev_y = 0
    #     y_new = []
    #     x_new = []
    #     i = 0
    #     for y in y_diffs:
    #         if -1 <= y[0] <= 1: #Filter Value is 1ms
    #             y_new.append(prev_y + y[0])
    #             x_new.append(x[i])
    #             prev_y = prev_y + y[0]
    #             i += 1                
    #     x = np.array(x_new).reshape((-1, 1))
    #     y = np.array(y_new)       
    #     model = LinearRegression().fit(x, y)
    #     #plt.plot(x, y, marker='.', markersize=0, linewidth='0.5', color='green')
    #     #plt.plot(x, model.coef_ * x + model.intercept_, color='orange', linestyle='--')
    #     #plt.show()
    #     for ind_srv in df[df['RSSI'] != 0 & (df['Destination Address'] == str(srv))].index:
    #         latency_cdc[ind_srv] = (latency[ind_srv] + df['Timestamp (us)'][ind_srv] * model.coef_)[0]
    #         latency_per_hop_cdc[ind_srv] = (latency_per_hop[ind_srv] + df['Timestamp (us)'][ind_srv] * model.coef_)[0]
             
            
    #Append Calculated Values
    df['Latency Time (ms)'] = latency
    df['Latency Time (ms) per Hop'] = latency_per_hop
    df['Throughput (B/s)'] = throughput
    df['Paket Loss %'] = pktloss
    # df['Latency Time (ms) cdc'] = latency_cdc
    # df['Latency Time (ms) per Hop cdc'] = latency_per_hop_cdc
    
    #Create Ongoing Transactions
    ongoing_transactions = [0 for i in range(len(df.index))]
    last_ongoing_transactions = 0
    for ind in df.index:
        #Is a Client Message
        if df['RSSI'][ind] == 0:
            #Update
            ongoing_transactions[ind] = last_ongoing_transactions + 1 * int(server_cnt[ind])
            last_ongoing_transactions = ongoing_transactions[ind]
        #Is a Server Message
        else:
            #Update
            if ongoing_transactions[ind] > 0:
                ongoing_transactions[ind] = last_ongoing_transactions - 1 
            last_ongoing_transactions = ongoing_transactions[ind]

    #Append Calculated Values
    df['Ongoing Transactions'] = ongoing_transactions

    #Clean and Calculate Average Values    
    df['Average Latency Time (ms) per Hop'] = [clean_and_calc_avg(latency_per_hop) for i in range(len(df.index))]
    df['Average Throughput (B/s)'] = [clean_and_calc_avg(throughput) for i in range(len(df.index))]
    df['Average Paket Loss %'] = [clean_and_calc_avg(pktloss) for i in range(len(df.index))]
    
    #Create Time Charts
    fig, axs = plt.subplots(4, 1)
    t_x = df['Timestamp (us)'].to_numpy()
    axs[0].plot(t_x, np.array(ongoing_transactions),drawstyle="steps-post")
    axs[0].set_xlim(df['Timestamp (us)'][0], df['Timestamp (us)'][len(df.index)-1])
    axs[0].set_xlabel('time us')
    axs[0].set_ylabel('Ongoing Transactions', fontsize=8)
    axs[0].grid(True)
    #Do Subplot
    axs[1].plot(t_x, np.array(latency_per_hop),drawstyle="steps-post")
    axs[1].set_xlim(df['Timestamp (us)'][0], df['Timestamp (us)'][len(df.index)-1])
    axs[1].set_xlabel('time us')
    axs[1].set_ylabel('Latency Time (ms) per Hop', fontsize=8)
    axs[1].grid(True)
    #Do Subplot
    axs[2].plot(t_x, np.array(throughput),drawstyle="steps-post")
    axs[2].set_xlim(df['Timestamp (us)'][0], df['Timestamp (us)'][len(df.index)-1])
    axs[2].set_xlabel('time us')
    axs[2].set_ylabel('Throughput (B/s)', fontsize=8)
    axs[2].grid(True)
    #Do Subplot
    axs[3].plot(t_x, np.array(pktloss),drawstyle="steps-post")
    axs[3].set_xlim(df['Timestamp (us)'][0], df['Timestamp (us)'][len(df.index)-1])
    axs[3].set_xlabel('time us')
    axs[3].set_ylabel('Paket Loss %', fontsize=8)
    axs[3].grid(True)
    #Do Double Axis Plot
##    axs2 = axs.twinx()
##    axs2.plot(t_x, np.array(latency),'r-',drawstyle="steps-post")
##    axs2.set_ylabel('Latency (ms)', color='r')
        
    #fig.tight_layout()
    fig.subplots_adjust(hspace=0)
    plt.show()
    
    return df
    

df = doAnalysis(df)

#Save File
df.to_csv(path,index=False,sep=';',encoding='utf-8') #For Europa use a Semicolon ; for America use , as seperator



                    
                    


