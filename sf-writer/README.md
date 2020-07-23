# sf-writer

## Data request ranges

Data request ranges are composed of:

- start_pulse_id (first pulse_id to be included in the file)
- stop_pulse_id (last pulse_id to be included in the file)
- pulse_id_step (how many pulses to skip between images.)

pulse_id_step can be used to write data at different frequencies:

- pulse_id_step == 1 (100Hz, write very pulse_id)
- pulse_id_step == 2 (50hz, write every second pulse)
- pulse_id_step == 10 (10Hz, write every 10th pulse)

The next pulse_id to be written is calculated internally as:

```c++
auto next_pulse_id = currnet_pulse_id + pulse_id_step;
```

The loop criteria for writing is:

```c++
for (
        auto curr_pulse_id = start_pulse_id; 
        curr_pulse_id <= stop_pulse_id;
        curr_pulse_id += pulse_id_step
) {
    // Write curr_pulse_id to output file.
}
```

**Warning**

If your stop_pulse_id cannot be reached by adding step_pulse_id to 
start_pulse_id (start_pulse_id + (n * pulse_id_step) != stop_pulse_id for any n)
it will not be included in the final file.