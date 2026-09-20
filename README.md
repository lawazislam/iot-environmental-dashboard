# IoT Environmental Monitoring & Analytics Dashboard

An end-to-end pipeline from embedded sensor simulation to an interactive Power BI dashboard: ESP32 + DHT22 (simulated in Wokwi) to a Python cleaning and feature-engineering pipeline to Power BI with DAX measures.

Solo project. Full report, with the KPI dashboard and complete 402-row dataset table: [`iot-environmental-dashboard.pdf`](https://lawazislam.com/assets/reports/iot-environmental-dashboard.pdf).

## Important: how the code in this repo was produced

None of the original source files survived. The report documents methodology and results in prose, plus the full 402-row output table, but contains no embedded code. Everything here was written from scratch afterward, and the two pieces were verified very differently, worth being precise about which is which:

**`data_pipeline.py` is fully verified, not just plausible.** The report never states the exact alert thresholds, so they were reverse-engineered: tested against the report's own published 402-row dataset until a rule was found that reproduces every single status label. The result: temperature above 26.0°C or humidity outside 35-45% flags a metric as out of range; both flagged at once is Critical, one is Warning, neither is Normal. Running this script against a raw version of the dataset (temperature and humidity only, none of the derived columns) reproduces the report's published numbers exactly: **402 readings, 25.43°C average, 41% in temperature alert, 27 critical events, 224/151/27 Normal/Warning/Critical**, matching to the decimal. Every field of every one of the 402 rows was diffed against the report's table: zero mismatches.

**`wokwi_sensor_firmware.ino` is a reconstruction, not a verified match.** There's no surviving raw serial log or original firmware to check this against, only the report's description (realistic, time-varying readings drifting from about 21°C to 29°C and 47% to 33% humidity over ~400 readings, with reading-to-reading noise). This file implements that described behavior, a slow drift toward a moving target blended with the DHT22's own reading plus small random noise, which is a standard technique for getting a Wokwi virtual sensor to produce a believable changing environment. It has not been run in Wokwi as part of this rebuild, and it will not reproduce your original dataset exactly (the real one presumably used a specific random seed or manual slider adjustments this can't recover). Treat it as a faithful behavioral match, not a recovered original.

## Files

- `wokwi_sensor_firmware.ino`: ESP32 + DHT22 firmware for Wokwi, outputs CSV over serial
- `data_pipeline.py`: cleans raw sensor CSV, adds 10-reading rolling averages, and classifies each reading as Normal, Warning, or Critical
- [`sample_raw_sensor_log.csv`](https://lawazislam.com/assets/data/iot-dashboard/sample_raw_sensor_log.csv): the report's real temperature/humidity readings, stripped down to only what the sensor stage would actually emit (used as the pipeline's test input). Hosted on my site rather than in this repo.
- [`processed_dataset_reference.csv`](https://lawazislam.com/assets/data/iot-dashboard/processed_dataset_reference.csv): the actual 402-row dataset as published in the report, used as the ground truth the pipeline was checked against. Also hosted on my site.

## Run it

```bash
pip install pandas numpy
curl -O https://lawazislam.com/assets/data/iot-dashboard/sample_raw_sensor_log.csv
python data_pipeline.py sample_raw_sensor_log.csv output.csv
```

Prints a summary (readings analyzed, average temperature, alert percentage, status breakdown) and writes the full processed dataset to `output.csv`. Compare it against [`processed_dataset_reference.csv`](https://lawazislam.com/assets/data/iot-dashboard/processed_dataset_reference.csv), it matches exactly.

## Tech stack

ESP32, DHT22, Wokwi, Python (pandas, numpy), Power BI, DAX
