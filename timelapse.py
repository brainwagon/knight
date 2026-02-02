import os
import subprocess
import datetime
import sys

def run_timelapse():
    # Configuration
    output_dir = "timelapse_frames"
    video_name = "day_night_cycle.mp4"
    interval_minutes = 5
    fps = 24
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Get current date
    now = datetime.datetime.utcnow()
    date_str = now.strftime("%Y-%m-%d")
    
    print(f"Starting timelapse generation for {date_str}...")
    print(f"Interval: {interval_minutes} minutes")
    
    frames = []
    # 24 hours * 60 minutes / interval
    total_steps = (24 * 60) // interval_minutes
    
    for i in range(total_steps):
        total_minutes = i * interval_minutes
        h = total_minutes // 60
        m = total_minutes % 60
        time_str = f"{h:02d}:{m:02d}:00"
        
        frame_filename = os.path.join(output_dir, f"frame_{i:04d}.pfm")
        png_filename = os.path.join(output_dir, f"frame_{i:04d}.png")
        
        # We use -c to get PNGs directly. 
        # We track the sun to see the movement across the sky
        cmd = [
            "./knight",
            "-d", date_str,
            "-t", time_str,
            "-w", "1280",
            "-h", "720",
            "-o", frame_filename,
            "-c", # Convert to PNG
            "--exposure", "1.0" # Slight boost for visibility
        ]
        
        # If you want a fixed view (e.g. looking South), uncomment these and remove tracking logic if desired
        cmd += ["-a", "20", "-z", "180"]
        
        print(f"[{i+1}/{total_steps}] Rendering {time_str}...", end="\r")
        sys.stdout.flush()
        
        result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        if result.returncode != 0:
            print(f"\nError rendering frame {i}: {result.stderr.decode()}")
            return

        # Clean up the PFM files to save space, keeping only PNGs
        if os.path.exists(frame_filename):
            os.remove(frame_filename)
            
    print("\nRendering complete. Assembling video with ffmpeg...")
    
    # FFmpeg command
    # -y: overwrite output
    # -framerate: input fps
    # -i: input pattern
    # -c:v: codec
    # -pix_fmt: pixel format for compatibility
    ffmpeg_cmd = [
        "ffmpeg", "-y",
        "-framerate", str(fps),
        "-i", os.path.join(output_dir, "frame_%04d.png"),
        "-c:v", "libx264",
        "-crf", "18",
        "-pix_fmt", "yuv420p",
        video_name
    ]
    
    result = subprocess.run(ffmpeg_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        print(f"Success! Video saved as {video_name}")
        print(f"To clean up frames, run: rm -rf {output_dir}")
    else:
        print(f"FFmpeg Error: {result.stderr.decode()}")

if __name__ == "__main__":
    run_timelapse()
