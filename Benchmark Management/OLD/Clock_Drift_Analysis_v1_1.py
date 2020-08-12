import os
import glob
import time
import datetime
import pandas as pd
import subprocess
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt


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

x = input("Plese type Number followed by enter to choose a file as Results file: ")
print(str(result[int(x)]) + " Will be loaded...")

#path = dirpath + "\\Analysis_Data_" + str(result[int(x)])
path = dirpath + "\\result_files\Analysis_Data_Test.csv"

df = pd.read_csv(result[int(x)],sep=';',encoding='utf-8')

def clean_and_calc_avg(ls):
    #Clean Data and Calculate Avg
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
    df_sorted = df.sort_values(by=['Timestamp (us)'], inplace=False) #Sort by Timestamp
    df_sorted = df_sorted.reset_index(drop=True) #Recreate Index
    #Timestamp Nulling
    offset_us = df_sorted['Timestamp (us)'][0]
    for ind in df_sorted.index:
        df_sorted['Timestamp (us)'][ind] = df_sorted['Timestamp (us)'][ind] - offset_us

    # replacing blank spaces with '_'  
    df_sorted.columns =[column.replace(" ", "_") for column in df_sorted.columns]

    latency = ['-' for i in range(len(df.index))]
    latency_per_hop = ['-' for i in range(len(df.index))]
    throughput = ['-' for i in range(len(df.index))]
    pkt_ok = ['-' for i in range(len(df.index))]
    pkt_loss_rate = ['-' for i in range(len(df.index))]
    ongoing_transactions = [0 for i in range(len(df.index))]
    last_ongoing_transactions = 0
    clock_drift_func_slope_a = ['-' for i in range(len(df.index))]
    clock_drift_func_intercept_b = ['-' for i in range(len(df.index))]
    latency_cdc = ['-' for i in range(len(df.index))]
    latency_per_hop_cdc = ['-' for i in range(len(df.index))]
    throughput_cdc = ['-' for i in range(len(df.index))]
    server_cnt = ['-' for i in range(len(df.index))]
    
    #Groups
    groups = df_sorted['Group_Address'].unique().tolist()
    for group in groups:
        #Clients
        clients = df_sorted.query('RSSI == "0" and Group_Address == "' + str(group) +'" ')['Source_Address'].unique().tolist()        
        for client in clients:
            #Servers        
            servers = df_sorted.query('RSSI != "0" and Group_Address == "' + str(group) +'" ')['Destination_Address'].unique().tolist()
            for server in servers:
                #Client Messages        
                client_messages = df_sorted.query('RSSI == "0" and Group_Address == "' + str(group) + '" and Source_Address == "' + str(client) +'"')['Message_ID'].unique().tolist()
                for client_message in client_messages:
                    #Process Message
                    client_message_df = df_sorted.query('RSSI == "0" and Group_Address == "' + str(group) + '" and Source_Address == "' + str(client) +'" and Message_ID == "' + str(client_message) + '"')
                    server_message_df = df_sorted.query('RSSI != "0" and Group_Address == "' + str(group) + '" and Destination_Address == "' + str(server) +'" and Message_ID == "' + str(client_message) + '"')
                    server_cnt[client_message_df['Timestamp_(us)'].index.item()] = len(servers)
                    if pkt_ok[client_message_df['Timestamp_(us)'].index.item()] == '-':
                            pkt_ok[client_message_df['Timestamp_(us)'].index.item()] = 0
                    if len(server_message_df.index) != 0:
                        latency[server_message_df['Timestamp_(us)'].index.item()] = (server_message_df['Timestamp_(us)'].item() - client_message_df['Timestamp_(us)'].item()) / 1000
                        latency_per_hop[server_message_df['Timestamp_(us)'].index.item()] = latency[server_message_df['Timestamp_(us)'].index.item()] / (server_message_df['Hops'].item() + 1)
                        throughput[server_message_df['Timestamp_(us)'].index.item()] = server_message_df['Data_Size'].item() / (latency[server_message_df['Timestamp_(us)'].index.item()] / 1000)
                        #ongoing_transactions[server_message_df['Timestamp_(us)'].index.item()] -= 1
                        if ongoing_transactions[server_message_df['Timestamp_(us)'].index.item()] < 0:
                            ongoing_transactions[server_message_df['Timestamp_(us)'].index.item()] = 0
                        pkt_ok[client_message_df['Timestamp_(us)'].index.item()] += 1
                    pkt_loss_rate[client_message_df['Timestamp_(us)'].index.item()] = (1 - (pkt_ok[client_message_df['Timestamp_(us)'].index.item()] / len(servers))) * 100
                

    #Append Data
    df_sorted['Latency_Time_(ms)'] = latency
    df_sorted['Latency_Time_(ms)_per_Hop'] = latency_per_hop
    df_sorted['Throughput_(B/s)'] = throughput
    df_sorted['Paket_Loss_%'] = pkt_loss_rate
    df_sorted['Subscribed Server Count'] = server_cnt

    #Create Ongoing Transactions
    ongoing_transactions = [0 for i in range(len(df_sorted.index))]
    last_ongoing_transactions = 0
    for ind in df_sorted.index:
        #Is a Client Message
        if df_sorted['RSSI'][ind] == 0:
            #Update
            ongoing_transactions[ind] = last_ongoing_transactions + 1 * int(df_sorted['Subscribed Server Count'][ind])
            last_ongoing_transactions = ongoing_transactions[ind]
        #Is a Server Message
        else:
            #Update
            if ongoing_transactions[ind] > 0:
                ongoing_transactions[ind] = last_ongoing_transactions - 1 
            last_ongoing_transactions = ongoing_transactions[ind]
    df_sorted['Ongoing_Transactions'] = ongoing_transactions

    
    
    def clean_and_calc_avg(ls):
        #Clean Data and Calculate Avg
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
     
    #Clean and Calculate Average Values    
    df_sorted['Average Latency Time (ms) per Hop'] = [clean_and_calc_avg(latency_per_hop) for i in range(len(df.index))]
    df_sorted['Average Throughput (B/s)'] = [clean_and_calc_avg(throughput) for i in range(len(df.index))]
    df_sorted['Average Latency Time (ms) per Hop CDC'] = [clean_and_calc_avg(latency_per_hop_cdc) for i in range(len(df.index))]
    df_sorted['Average Throughput (B/s) CDC'] = [clean_and_calc_avg(throughput_cdc) for i in range(len(df.index))]
    df_sorted['Average Paket Loss %'] = [clean_and_calc_avg(pkt_loss_rate) for i in range(len(df.index))]
    
    #Create Time Charts
    fig, axs = plt.subplots(4, 1)
    t_x = df_sorted['Timestamp_(us)'].to_numpy()
    axs[0].plot(t_x, np.array(ongoing_transactions),drawstyle="steps-post")
    axs[0].set_xlim(df_sorted['Timestamp_(us)'][0], df_sorted['Timestamp_(us)'][len(df.index)-1])
    axs[0].set_xlabel('time us')
    axs[0].set_ylabel('Ongoing Transactions', fontsize=8)
    axs[0].grid(True)
    #Do Subplot
    axs[1].plot(t_x, np.array(latency_per_hop), marker='.', markersize=0, linewidth='0.5', color='green',drawstyle="steps-post")
    axs[1].plot(t_x, np.array(latency_per_hop_cdc), color='orange', linestyle='--' ,drawstyle="steps-post")
    #axs[1].plot(t_x, np.array(latency_per_hop),drawstyle="steps-post")
    axs[1].set_xlim(df_sorted['Timestamp_(us)'][0], df_sorted['Timestamp_(us)'][len(df.index)-1])
    axs[1].set_xlabel('time us')
    axs[1].set_ylabel('Latency Time (ms) per Hop', fontsize=8)
    axs[1].grid(True)
    #Do Subplot
    axs[2].plot(t_x, np.array(throughput),drawstyle="steps-post")
    axs[2].set_xlim(df_sorted['Timestamp_(us)'][0], df_sorted['Timestamp_(us)'][len(df.index)-1])
    axs[2].set_xlabel('time us')
    axs[2].set_ylabel('Throughput (B/s)', fontsize=8)
    axs[2].grid(True)
    #Do Subplot
    axs[3].plot(t_x, np.array(pkt_loss_rate),drawstyle="steps-post")
    axs[3].set_xlim(df_sorted['Timestamp_(us)'][0], df_sorted['Timestamp_(us)'][len(df.index)-1])
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
 
    
    

        

   
    


    
    
    

df = doAnalysis(df)

#Save File
#df.to_csv(path,index=False,sep=';',encoding='utf-8') #For Europa use a Semicolon ; for America use , as seperator



                    
                    


