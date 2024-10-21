import matplotlib.pyplot as plt

# New system_ticks for faster intervals
system_ticks = [0, 500, 1000, 1500, 2000, 2500]

# Keep the same or adjust process ticks based on your scheduler's results
process_1_ticks = [0, 70, 130, 180, 220, 250]   # Process 1 (10 tickets)
process_2_ticks = [0, 150, 260, 370, 460, 550]  # Process 2 (20 tickets)
process_3_ticks = [0, 250, 450, 660, 880, 1100] # Process 3 (30 tickets)

# Plot as before
fig, ax = plt.subplots()

ax.plot(system_ticks, process_1_ticks, 'b-', label='Process 1 (10 tickets)', linewidth=2)
ax.plot(system_ticks, process_2_ticks, 'r-', label='Process 2 (20 tickets)', linewidth=2)
ax.plot(system_ticks, process_3_ticks, 'g-', label='Process 3 (30 tickets)', linewidth=2)

ax.legend()
ax.set_xlabel('System Time (ticks)')
ax.set_ylabel('Process Time Slices (ticks)')
ax.set_title('Lottery Scheduling: Time Slices Allocated to Processes')

plt.savefig('lottery_scheduler_results_new.png')

