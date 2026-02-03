#!/usr/bin/env python3
"""
Pokemon Sprite Converter GUI - FIXED VERSION
Converts PNG sprites to GBA format, avoiding grit symbol name collisions

CRITICAL FIX: Adds 'p' prefix to avoid collisions:
- 0017.png and 1017.png would both create _017 (COLLISION!)
- p0017.png creates _p0017 and p1017.png creates _p1017 (NO COLLISION!)
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import subprocess
import os
import sys
from pathlib import Path
from PIL import Image

class SpriteConverterGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Pokemon Sprite Converter - Fixed Version")
        self.root.geometry("800x700")
        
        # Variables
        self.input_folder = tk.StringVar()
        self.output_folder = tk.StringVar()
        self.target_size = tk.StringVar(value="64")
        self.resize_filter = tk.StringVar(value="LANCZOS")
        
        self.create_widgets()
        
    def create_widgets(self):
        # Title
        title = tk.Label(self.root, text="Pokemon Sprite Converter (Fixed)", 
                        font=("Arial", 16, "bold"))
        title.pack(pady=10)
        
        # Warning label
        warning = tk.Label(self.root, 
                          text="⚠️ This version adds 'p' prefix to avoid symbol collisions",
                          fg="red", font=("Arial", 10, "bold"))
        warning.pack(pady=5)
        
        # Input folder
        input_frame = tk.Frame(self.root)
        input_frame.pack(fill="x", padx=20, pady=5)
        
        tk.Label(input_frame, text="Input Folder (PNG sprites):", 
                width=25, anchor="w").pack(side="left")
        tk.Entry(input_frame, textvariable=self.input_folder, 
                width=40).pack(side="left", padx=5)
        tk.Button(input_frame, text="Browse...", 
                 command=self.browse_input).pack(side="left")
        
        # Output folder
        output_frame = tk.Frame(self.root)
        output_frame.pack(fill="x", padx=20, pady=5)
        
        tk.Label(output_frame, text="Output Folder (GBA .c/.h):", 
                width=25, anchor="w").pack(side="left")
        tk.Entry(output_frame, textvariable=self.output_folder, 
                width=40).pack(side="left", padx=5)
        tk.Button(output_frame, text="Browse...", 
                 command=self.browse_output).pack(side="left")
        
        # Size selector
        size_frame = tk.Frame(self.root)
        size_frame.pack(fill="x", padx=20, pady=5)
        
        tk.Label(size_frame, text="Target Size (for GBA):", 
                width=25, anchor="w").pack(side="left")
        size_combo = ttk.Combobox(size_frame, textvariable=self.target_size,
                                  values=["32", "48", "64", "96", "128"],
                                  state="readonly", width=10)
        size_combo.pack(side="left", padx=5)
        tk.Label(size_frame, text="(Recommended: 64x64)", 
                fg="blue").pack(side="left")
        
        # Resize filter
        filter_frame = tk.Frame(self.root)
        filter_frame.pack(fill="x", padx=20, pady=5)
        
        tk.Label(filter_frame, text="Resize Quality:", 
                width=25, anchor="w").pack(side="left")
        filter_combo = ttk.Combobox(filter_frame, textvariable=self.resize_filter,
                                    values=["LANCZOS", "BICUBIC", "BILINEAR", "NEAREST"],
                                    state="readonly", width=10)
        filter_combo.pack(side="left", padx=5)
        tk.Label(filter_frame, text="(LANCZOS = best quality)", 
                fg="blue").pack(side="left")
        
        # Info box
        info_frame = tk.LabelFrame(self.root, text="What This Does", padx=10, pady=10)
        info_frame.pack(fill="x", padx=20, pady=10)
        
        info_text = """1. Renames PNGs with 'p' prefix (0017.png → p0017.png) to avoid collisions
2. Resizes sprites to target size using high-quality filter
3. Converts to GBA format using grit
4. Creates _p0017Tiles, _p1017Tiles symbols (NO COLLISIONS!)
5. Cleans up temporary files"""
        
        tk.Label(info_frame, text=info_text, justify="left", 
                font=("Courier", 9)).pack()
        
        # Convert button
        convert_btn = tk.Button(self.root, text="🚀 Convert All Sprites", 
                               command=self.convert_sprites,
                               font=("Arial", 12, "bold"),
                               bg="#4CAF50", fg="white",
                               height=2)
        convert_btn.pack(pady=15)
        
        # Log output
        log_frame = tk.LabelFrame(self.root, text="Conversion Log")
        log_frame.pack(fill="both", expand=True, padx=20, pady=10)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=15, 
                                                   font=("Courier", 9))
        self.log_text.pack(fill="both", expand=True, padx=5, pady=5)
        
    def browse_input(self):
        folder = filedialog.askdirectory(title="Select Input Folder with PNG Sprites")
        if folder:
            self.input_folder.set(folder)
            self.log(f"✓ Input folder: {folder}")
            
    def browse_output(self):
        folder = filedialog.askdirectory(title="Select Output Folder for GBA Files")
        if folder:
            self.output_folder.set(folder)
            self.log(f"✓ Output folder: {folder}")
    
    def log(self, message):
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.root.update()
        
    def convert_sprites(self):
        input_dir = self.input_folder.get()
        output_dir = self.output_folder.get()
        target_size = int(self.target_size.get())
        
        if not input_dir or not output_dir:
            messagebox.showerror("Error", "Please select input and output folders!")
            return
        
        if not os.path.exists(input_dir):
            messagebox.showerror("Error", "Input folder doesn't exist!")
            return
        
        # Create output folder
        os.makedirs(output_dir, exist_ok=True)
        
        # Create temp folder for resized images
        temp_dir = os.path.join(output_dir, "temp_resized")
        os.makedirs(temp_dir, exist_ok=True)
        
        self.log("\n" + "="*60)
        self.log("🎮 STARTING POKEMON SPRITE CONVERSION")
        self.log("="*60)
        self.log(f"Input: {input_dir}")
        self.log(f"Output: {output_dir}")
        self.log(f"Target size: {target_size}x{target_size}")
        self.log(f"Resize filter: {self.resize_filter.get()}")
        self.log("")
        
        # Get resize filter
        filter_name = self.resize_filter.get()
        resize_filter = getattr(Image.Resampling, filter_name)
        
        # Find all PNG files (case-insensitive)
        png_files = sorted([f for f in os.listdir(input_dir) 
                           if f.lower().endswith('.png')])
        
        if not png_files:
            self.log("❌ No PNG files found!")
            messagebox.showerror("Error", "No PNG files found in input folder!")
            return
        
        self.log(f"Found {len(png_files)} PNG files")
        self.log("")
        
        converted = 0
        errors = []
        
        for i, png_file in enumerate(png_files, 1):
            try:
                # Get original filename without extension
                name_without_ext = os.path.splitext(png_file)[0]
                
                # Add 'p' prefix if not already present
                if not name_without_ext.startswith('p'):
                    new_name = f"p{name_without_ext}"
                else:
                    new_name = name_without_ext
                
                input_path = os.path.join(input_dir, png_file)
                temp_path = os.path.join(temp_dir, f"{new_name}.png")
                
                # Open and check source image
                img = Image.open(input_path)
                original_size = img.size
                
                # Resize if needed
                if img.size != (target_size, target_size):
                    img = img.resize((target_size, target_size), resize_filter)
                
                # Composite onto RGB background, then quantize to indexed.
                # Order matters: quantizing RGBA directly embeds a tRNS
                # transparency chunk that grit rejects (outputs "palette 0
                # entries"). Flattening to RGB first produces a clean
                # indexed PNG with no tRNS that grit reads correctly.
                if img.mode in ("RGBA", "LA", "PA"):
                    background = Image.new("RGB", img.size, (0, 0, 0))
                    background.paste(img, mask=img.split()[-1])  # composite using alpha
                    img = background.quantize(colors=256, method=Image.Quantize.MEDIANCUT)
                elif img.mode != "P":
                    img = img.quantize(colors=256, method=Image.Quantize.MEDIANCUT)
                # else already indexed, leave alone
                
                # Save to temp folder — clean indexed PNG, no tRNS
                img.save(temp_path)
                
                # Convert with grit
                grit_cmd = [
                    "grit",
                    temp_path,
                    "-ftc",       # output as C source
                    "-gB8",       # 8-bit graphics (indexed)
                    "-p",         # palette inline in .c
                    f"-o{output_dir}/{new_name}"
                ]
                
                result = subprocess.run(grit_cmd, capture_output=True, text=True)
                
                if result.returncode != 0:
                    errors.append(f"{png_file}: {result.stderr}")
                    self.log(f"❌ Error converting {png_file}")
                else:
                    converted += 1
                    
                    if converted % 100 == 0:
                        self.log(f"✓ Converted {converted}/{len(png_files)} sprites...")
                
            except Exception as e:
                errors.append(f"{png_file}: {str(e)}")
                self.log(f"❌ Error: {png_file} - {str(e)}")
        
        # Clean up temp folder
        self.log("")
        self.log("Cleaning up temporary files...")
        try:
            import shutil
            shutil.rmtree(temp_dir)
            self.log("✓ Temp folder deleted")
        except:
            self.log("⚠️ Could not delete temp folder (not critical)")
        
        # Summary
        self.log("")
        self.log("="*60)
        self.log(f"✅ CONVERSION COMPLETE!")
        self.log(f"   Converted: {converted}/{len(png_files)} files")
        if errors:
            self.log(f"   Errors: {len(errors)}")
            self.log("\nFirst few errors:")
            for err in errors[:5]:
                self.log(f"   - {err}")
        self.log("="*60)
        self.log("")
        self.log("📝 Next Steps:")
        self.log("1. Copy converted .c files to your Build/gfx/sprites/ folder")
        self.log("2. Run the sprite_lookup generator script")
        self.log("3. Rebuild your project")
        
        messagebox.showinfo("Complete", 
                           f"Converted {converted}/{len(png_files)} sprites!\n\n"
                           f"Check log for details.")

if __name__ == "__main__":
    root = tk.Tk()
    app = SpriteConverterGUI(root)
    root.mainloop()
