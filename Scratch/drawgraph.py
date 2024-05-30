import os
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np


attributes = {
    "nDevices" : 182,
    "period" : 30,
    "GateWayCordinate" : [4000,4000]
}

# Temel klasör yolu
#base_folder = 'experiments/Logs_period-30_nDevices-182'
base_folder = 'experiments/Logs_period-'+str(attributes["period"])+'_nDevices-' + str(attributes["nDevices"])

# Regex pattern'leri
delay_pattern = re.compile(r'Average Delay:\s+(\d+\.\d+)')
pdr_pattern = re.compile(r'PDR:\s+(\d+\.\d+)%')

# Sonuçları saklamak için listeler
delays = []
pdrs = []

# Klasörleri sıralı olarak almak için
run_folders = sorted(os.listdir(base_folder), key=lambda x: int(x.replace('run', '').strip()))

# Belirtilen klasördeki her bir run klasörü için işlemleri yap
for run_folder in run_folders:
    run_path = os.path.join(base_folder, run_folder)
    if os.path.isdir(run_path):
        log_file_path = os.path.join(run_path, 'log.txt')
        if os.path.exists(log_file_path):
            with open(log_file_path, 'r') as log_file:
                log_content = log_file.read()
                
                # Delay değerini çek
                delay_match = delay_pattern.search(log_content)
                if delay_match:
                    delay_value = float(delay_match.group(1))
                    delays.append(delay_value)
                else:
                    delays.append(None)
                
                # PDR değerini çek
                pdr_match = pdr_pattern.search(log_content)
                if pdr_match:
                    pdr_value = float(pdr_match.group(1))
                    pdrs.append(pdr_value)
                else:
                    pdrs.append(None)

# Ortalama değerlerin hesaplanması
avg_delay = np.nanmean(delays)  # NaN değerleri hesaba katmadan ortalama al
avg_pdr = np.nanmean(pdrs)      # NaN değerleri hesaba katmadan ortalama al

# Grafik oluşturma
fig, axes = plt.subplots(2, 1, figsize=(10, 10))

# Average Delay grafiği
sns.lineplot(ax=axes[0], x=range(1, len(delays) + 1), y=delays, marker='o')
axes[0].set_title('Average Delay per Run')
axes[0].set_xlabel('Run')
axes[0].set_ylabel('Average Delay (s)')
axes[0].axhline(y=avg_delay, color='r', linestyle='--', label=f'Average: {avg_delay:.2f}')
axes[0].legend()
 # Y ekseni sınırları

# PDR grafiği
sns.lineplot(ax=axes[1], x=range(1, len(pdrs) + 1), y=pdrs, marker='o')
axes[1].set_title('PDR per Run')
axes[1].set_xlabel('Run')
axes[1].set_ylabel('PDR (%)')
axes[1].axhline(y=avg_pdr, color='r', linestyle='--', label=f'Average: {avg_pdr:.2f}')
axes[1].legend()

# Grafiklerin düzenlenmesi
plt.tight_layout(rect=[0, 0.05, 1, 1])  # Alt kısımda biraz boşluk bırak

fig.text(0.5, 0.01, f"Period: {attributes['period']} | nDevices: {attributes['nDevices']} | GateWayCordinates: {attributes.get('GateWayCordinate', 'N/A')}", ha='center')

plt.savefig("graph-period-"+str(attributes["period"])+"_nDevices-" + str(attributes["nDevices"]))
plt.show()

