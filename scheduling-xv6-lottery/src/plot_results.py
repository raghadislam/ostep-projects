import matplotlib.pyplot as plt

# Example data (replace with your collected data)
process_ids = ['Process A', 'Process B', 'Process C']
ticks = [30, 20, 10]  # Replace with actual tick values from results

plt.bar(process_ids, ticks, color=['blue', 'orange', 'green'])
plt.xlabel('Processes')
plt.ylabel('Time Slices Allocated')
plt.title('Lottery Scheduler Time Slice Allocation')
plt.ylim(0, max(ticks) + 5)  # Adjust y-axis for better visibility
plt.grid(axis='y')

# Show the plot
plt.show()

