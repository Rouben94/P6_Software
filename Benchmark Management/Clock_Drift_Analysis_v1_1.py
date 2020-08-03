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
                clock_drift_func_prev = 0
                clock_drift_last_diff = 0
                clock_drift_func = []
                clock_drift_func_time = []
                #Client Messages        
                client_messages = df_sorted.query('RSSI == "0" and Group_Address == "' + str(group) + '" and Source_Address == "' + str(client) +'"')['Message_ID'].unique().tolist()
                for client_message in client_messages:
                    #Process Message
                    client_message_df = df_sorted.query('RSSI == "0" and Group_Address == "' + str(group) + '" and Source_Address == "' + str(client) +'" and Message_ID == "' + str(client_message) + '"')
                    server_message_df = df_sorted.query('RSSI != "0" and Group_Address == "' + str(group) + '" and Destination_Address == "' + str(server) +'" and Message_ID == "' + str(client_message) + '"')
                    if pkt_ok[client_message_df['Timestamp_(us)'].index.item()] == '-':
                            pkt_ok[client_message_df['Timestamp_(us)'].index.item()] = 0
                    if len(server_message_df.index) != 0:
                        latency[server_message_df['Timestamp_(us)'].index.item()] = (server_message_df['Timestamp_(us)'].item() - client_message_df['Timestamp_(us)'].item()) / 1000
                        latency_per_hop[server_message_df['Timestamp_(us)'].index.item()] = latency[server_message_df['Timestamp_(us)'].index.item()] / (server_message_df['Hops'].item() + 1)
                        diff = latency_per_hop[server_message_df['Timestamp_(us)'].index.item()] /1e3 - clock_drift_func_prev
                        if -1e-3 <= diff <= 1e-3: #Filter Value is 1ms
                            clock_drift_func.append(clock_drift_last_diff + diff)
                            clock_drift_func_time.append(server_message_df['Timestamp_(us)'].item() / 1e6)
                            clock_drift_last_diff += diff
                        clock_drift_func_prev = latency_per_hop[server_message_df['Timestamp_(us)'].index.item()] / 1e3
                        throughput[server_message_df['Timestamp_(us)'].index.item()] = server_message_df['Data_Size'].item() / (latency[server_message_df['Timestamp_(us)'].index.item()] / 1000)
                        pkt_ok[client_message_df['Timestamp_(us)'].index.item()] += 1
                    pkt_loss_rate[client_message_df['Timestamp_(us)'].index.item()] = (1 - (pkt_ok[client_message_df['Timestamp_(us)'].index.item()] / len(servers))) * 100
                #Clock Drift Function                       
                print(clock_drift_func)
                x = np.array(clock_drift_func_time).reshape((-1, 1))
                #x = np.array(list(range(0,len(clock_drift_func)))).reshape((-1, 1))
                y = np.array(clock_drift_func)
                model = LinearRegression().fit(x, y)
                print('slope:', model.coef_) # ppm (us/s)
                print('intercept:', model.intercept_)
                plt.plot(x, y, marker='.', markersize=0, linewidth='0.5', color='green')
                plt.plot(x, model.coef_ * x + model.intercept_, color='orange', linestyle='--')
                plt.show()
                
    print(groups)
    
    
    

        


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
    
    


    
    
    

df = doAnalysis(df)

#Save File
#df.to_csv(path,index=False,sep=';',encoding='utf-8') #For Europa use a Semicolon ; for America use , as seperator



                    
                    


