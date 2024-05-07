docs hosted at :  [doxygen documented link](https://amrith-h-n.github.io/water_monitoring_g6_base_station/)
# initialize workspace for the water monitoring base station (main branch)

```
west init -m https://github.com/Amrith-H-N/water_monitoring_g6_base_station --mr main water_monitoring_g6_base_station
```
or to init exising directory (inside the directory with west.yml)
```
west init -l 
west update
west build -b rpi_pico -s app -p
```

# update Zephyr modules
```
cd water_monitoring_g6_base_station
west update
```
# build app
```
cd water_monitoring_g6_base_station
west build -b rpi_pico -s app -p
```

# flash app
```
west flash
```
