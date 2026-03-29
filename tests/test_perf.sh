#!/bin/bash
set -euo pipefail

EXE="./build/rubiks-cube"
CONFIG="/home/walter/.rubiks-cube/config.ini"
DURATION=15
RESULTS="/tmp/perf_results.txt"
CPU_RAW="/tmp/cpu_raw.txt"
GPU_RAW="/tmp/gpu_raw.txt"

cp "$CONFIG" "${CONFIG}.bak"
rm -f "$RESULTS" "$CPU_RAW" "$GPU_RAW"

avg_nums() {
    local sum=0 cnt=0
    for v in $*; do
        sum=$(echo "$sum + $v" | bc)
        cnt=$((cnt + 1))
    done
    [ $cnt -gt 0 ] && echo "scale=1; $sum / $cnt" | bc || echo "N/A"
}

max_num() {
    local m=0
    for v in $*; do
        (( $(echo "$v > $m" | bc) )) && m=$v
    done
    echo "$m"
}

min_num() {
    local m="999999"
    for v in $*; do
        (( $(echo "$v < $m" | bc) )) && m=$v
    done
    echo "$m"
}

run_test() {
    local rtype="$1"
    local label="$2"

    echo "" | tee -a "$RESULTS"
    echo "===== $label =====" | tee -a "$RESULTS"

    sed -i "s/^rendererType.*/rendererType = $rtype/" "$CONFIG"

    rm -f "$CPU_RAW" "$GPU_RAW"

    "$EXE" --scramble --celebrate &
    local pid=$!
    sleep 2

    if ! kill -0 $pid 2>/dev/null; then
        echo "ERROR: process exited early" | tee -a "$RESULTS"
        return
    fi

    echo "Measuring for ${DURATION}s..." | tee -a "$RESULTS"

    sudo timeout $((DURATION + 2))s intel_gpu_top -s 500 > "$GPU_RAW" 2>/dev/null &
    local gpu_pid=$!

    top -b -n $((DURATION + 1)) -d 1 -p "$pid" > "$CPU_RAW" 2>/dev/null

    wait $gpu_pid 2>/dev/null || true

    local cpu_list=""
    while IFS= read -r line; do
        local c
        c=$(echo "$line" | awk '{print $9}')
        if [[ "$c" =~ ^[0-9.]+$ ]] && [ "$c" != "0.0" ]; then
            cpu_list="$cpu_list $c"
        fi
    done < <(awk "/^ *$pid /" "$CPU_RAW")

    local gpu_list=""
    while IFS= read -r line; do
        local val
        val=$(echo "$line" | awk '{gsub(",","."); print $7}')
        if [[ "$val" =~ ^[0-9.]+$ ]]; then
            gpu_list="$gpu_list $val"
        fi
    done < <(tail -n +3 "$GPU_RAW" | grep -v '^[[:space:]]*$' | grep -v '^$')

    local cpu_avg gpu_avg cpu_min cpu_max gpu_min gpu_max
    cpu_avg=$(avg_nums $cpu_list)
    cpu_min=$(min_num $cpu_list)
    cpu_max=$(max_num $cpu_list)
    gpu_avg=$(avg_nums $gpu_list)
    gpu_min=$(min_num $gpu_list)
    gpu_max=$(max_num $gpu_list)

    local cpu_cnt gpu_cnt
    cpu_cnt=$(echo "$cpu_list" | wc -w)
    gpu_cnt=$(echo "$gpu_list" | wc -w)

    echo "  CPU: n=$cpu_cnt min=${cpu_min}% avg=${cpu_avg}% max=${cpu_max}%" | tee -a "$RESULTS"
    echo "  GPU: n=$gpu_cnt min=${gpu_min}% avg=${gpu_avg}% max=${gpu_max}%" | tee -a "$RESULTS"
    echo "$label | CPU_avg=${cpu_avg}% (min=${cpu_min}%,max=${cpu_max}%) | GPU_avg=${gpu_avg}% (min=${gpu_min}%,max=${gpu_max}%)" | tee -a "$RESULTS"

    kill $pid 2>/dev/null || true
    wait $pid 2>/dev/null || true
    sleep 1.0
}

echo "=== Performance test: $(date) ===" | tee "$RESULTS"

run_test 0 "OpenGL"
run_test 1 "Shader-instanced"

cp "${CONFIG}.bak" "$CONFIG"

echo "" | tee -a "$RESULTS"
echo "===== SUMMARY =====" | tee -a "$RESULTS"
grep 'OpenGL\|Shader' "$RESULTS"
