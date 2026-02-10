#!/usr/bin/env python3
"""
Pokemon Sprite Converter with AUTO-RESIZE
Resizes 192x192 PNGs to 64x64 using Pillow, then converts to GBA
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import os
import sys
from PIL import Image
import subprocess
from pathlib import Path
import tempfile
import shutil

class SpriteConverter:
    def __init__(self, root):
        self.root = root
        self.root.title("Pokemon Sprite Converter with AUTO-RESIZE")
        self.root.geometry("900x750")
        
        self.sprites_converted = 0
        self.total_sprites = 0
        self.grit_path = None
        self.temp_dir = None
        
        self.create_widgets()
        self.check_grit()
    
    def create_widgets(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Title
        title = ttk.Label(main_frame, text="Pokemon Sprite Converter + Auto-Resize", 
                         font=('Arial', 14, 'bold'), foreground='blue')
        title.grid(row=0, column=0, columnspan=3, pady=10)
        
        # Input folder
        ttk.Label(main_frame, text="Input Folder (PNG sprites):").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.input_entry = ttk.Entry(main_frame, width=50)
        self.input_entry.grid(row=1, column=1, sticky=(tk.W, tk.E), pady=5)
        ttk.Button(main_frame, text="Browse", command=self.browse_input).grid(row=1, column=2, padx=5, pady=5)
        
        # Output folder
        ttk.Label(main_frame, text="Output Folder (GBA files):").grid(row=2, column=0, sticky=tk.W, pady=5)
        self.output_entry = ttk.Entry(main_frame, width=50)
        self.output_entry.grid(row=2, column=1, sticky=(tk.W, tk.E), pady=5)
        ttk.Button(main_frame, text="Browse", command=self.browse_output).grid(row=2, column=2, padx=5, pady=5)
        
        # Settings frame
        settings_frame = ttk.LabelFrame(main_frame, text="Conversion Settings", padding="10")
        settings_frame.grid(row=3, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=10)
        
        # Size with AUTO-RESIZE notice
        ttk.Label(settings_frame, text="Output Size:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.size_var = tk.StringVar(value="64x64")
        size_combo = ttk.Combobox(settings_frame, textvariable=self.size_var, width=15)
        size_combo['values'] = ('32x32', '64x64', '96x96', '128x128')
        size_combo.grid(row=0, column=1, sticky=tk.W, pady=5)
        ttk.Label(settings_frame, text="← PNGs will be RESIZED to this!", 
                 foreground='blue', font=('Arial', 9, 'bold')).grid(row=0, column=2, sticky=tk.W, padx=10)
        
        # Resize method
        ttk.Label(settings_frame, text="Resize Quality:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.resize_method_var = tk.StringVar(value="LANCZOS")
        resize_combo = ttk.Combobox(settings_frame, textvariable=self.resize_method_var, width=15)
        resize_combo['values'] = ('LANCZOS', 'BICUBIC', 'BILINEAR', 'NEAREST')
        resize_combo.grid(row=1, column=1, sticky=tk.W, pady=5)
        ttk.Label(settings_frame, text="(LANCZOS = best quality)").grid(row=1, column=2, sticky=tk.W, padx=10)
        
        # Color depth
        ttk.Label(settings_frame, text="Color Depth:").grid(row=2, column=0, sticky=tk.W, pady=5)
        self.color_var = tk.StringVar(value="8-bit (256 colors)")
        color_combo = ttk.Combobox(settings_frame, textvariable=self.color_var, width=20)
        color_combo['values'] = ('4-bit (16 colors)', '8-bit (256 colors)')
        color_combo.grid(row=2, column=1, sticky=tk.W, pady=5)
        
        # Transparent color
        ttk.Label(settings_frame, text="Transparent Color:").grid(row=3, column=0, sticky=tk.W, pady=5)
        self.trans_var = tk.StringVar(value="#FF00FF")
        trans_entry = ttk.Entry(settings_frame, textvariable=self.trans_var, width=10)
        trans_entry.grid(row=3, column=1, sticky=tk.W, pady=5)
        
        # Compression
        self.compress_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(settings_frame, text="Enable LZ77 Compression", 
                       variable=self.compress_var).grid(row=4, column=0, columnspan=2, sticky=tk.W, pady=5)
        
        # Progress
        progress_frame = ttk.Frame(main_frame)
        progress_frame.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=10)
        
        self.progress_label = ttk.Label(progress_frame, text="Ready")
        self.progress_label.pack(anchor=tk.W)
        
        self.progress_bar = ttk.Progressbar(progress_frame, length=400, mode='determinate')
        self.progress_bar.pack(fill=tk.X, pady=5)
        
        # Buttons
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=5, column=0, columnspan=3, pady=10)
        
        ttk.Button(button_frame, text="Convert All Sprites", 
                  command=self.convert_all, width=20).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Clear Log", 
                  command=self.clear_log, width=15).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Exit", 
                  command=self.root.quit, width=10).pack(side=tk.LEFT, padx=5)
        
        # Log
        log_frame = ttk.LabelFrame(main_frame, text="Conversion Log", padding="5")
        log_frame.grid(row=6, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=15, width=100)
        self.log_text.pack(fill=tk.BOTH, expand=True)
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(6, weight=1)
    
    def check_grit(self):
        """Check if grit is available"""
        try:
            result = subprocess.run(['grit', '--version'], 
                                  capture_output=True, text=True, timeout=5)
            self.grit_path = 'grit'
            self.log("✓ Grit found and ready")
        except:
            self.log("⚠ Grit not found in PATH")
            self.grit_path = None
    
    def browse_input(self):
        folder = filedialog.askdirectory(title="Select PNG Sprites Folder")
        if folder:
            self.input_entry.delete(0, tk.END)
            self.input_entry.insert(0, folder)
            
            # Count PNG files and check sizes
            png_files = [f for f in os.listdir(folder) if f.lower().endswith('.png')]
            if png_files:
                # Check first file size
                first_file = os.path.join(folder, png_files[0])
                img = Image.open(first_file)
                self.log(f"Found {len(png_files)} PNG files")
                self.log(f"Current size: {img.width}x{img.height}")
                self.log(f"Will be resized to: {self.size_var.get()}")
    
    def browse_output(self):
        folder = filedialog.askdirectory(title="Select Output Folder")
        if folder:
            self.output_entry.delete(0, tk.END)
            self.output_entry.insert(0, folder)
    
    def log(self, message):
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.root.update()
    
    def clear_log(self):
        self.log_text.delete(1.0, tk.END)
    
    def resize_png(self, input_path, output_path, target_size):
        """Resize PNG using Pillow"""
        try:
            img = Image.open(input_path)
            
            # Get resize method
            resize_method_map = {
                'LANCZOS': Image.Resampling.LANCZOS,
                'BICUBIC': Image.Resampling.BICUBIC,
                'BILINEAR': Image.Resampling.BILINEAR,
                'NEAREST': Image.Resampling.NEAREST
            }
            method = resize_method_map.get(self.resize_method_var.get(), Image.Resampling.LANCZOS)
            
            # Resize
            img_resized = img.resize(target_size, method)
            
            # Save
            img_resized.save(output_path, 'PNG')
            return True
        except Exception as e:
            return False
    
    def convert_with_grit(self, input_path, output_path, sprite_name):
        """Convert sprite using grit tool"""
        cmd = [self.grit_path, input_path]
        
        # Color depth
        if '4-bit' in self.color_var.get():
            cmd.append('-gB4')
        else:
            cmd.append('-gB8')
        
        # Transparent color
        trans_color = self.trans_var.get().replace('#', '')
        cmd.append(f'-gT{trans_color}')
        
        # Output format
        cmd.extend(['-ftc', '-fh!'])
        
        # Compression
        if self.compress_var.get():
            cmd.append('-gz')
        
        # Palette
        cmd.append('-pS')
        
        # Output path
        output_base = os.path.join(output_path, sprite_name)
        cmd.extend(['-o', output_base])
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
            if result.returncode == 0:
                return True, "Success"
            else:
                return False, f"Grit error: {result.stderr[:100]}"
        except Exception as e:
            return False, f"Error: {str(e)}"
    
    def convert_all(self):
        input_folder = self.input_entry.get()
        output_folder = self.output_entry.get()
        
        if not input_folder or not os.path.isdir(input_folder):
            messagebox.showerror("Error", "Please select a valid input folder!")
            return
        
        if not output_folder:
            messagebox.showerror("Error", "Please select an output folder!")
            return
        
        if not self.grit_path:
            messagebox.showerror("Error", "Grit not found! Please run from MSYS terminal.")
            return
        
        # Create output folder
        os.makedirs(output_folder, exist_ok=True)
        
        # Create temp folder for resized PNGs
        self.temp_dir = tempfile.mkdtemp(prefix="pokemon_resize_")
        self.log(f"Created temp folder: {self.temp_dir}")
        
        # Get PNG files
        png_files = [f for f in os.listdir(input_folder) if f.lower().endswith('.png')]
        
        if not png_files:
            messagebox.showerror("Error", "No PNG files found!")
            return
        
        self.total_sprites = len(png_files)
        self.sprites_converted = 0
        
        # Get target size
        size_str = self.size_var.get()
        target_width = int(size_str.split('x')[0])
        target_size = (target_width, target_width)
        
        self.log(f"\n{'='*60}")
        self.log(f"Starting conversion of {self.total_sprites} sprites")
        self.log(f"Input size: 192x192 (or whatever source is)")
        self.log(f"Output size: {size_str} (RESIZED!)")
        self.log(f"Resize method: {self.resize_method_var.get()}")
        self.log(f"Color depth: {self.color_var.get()}")
        self.log(f"Compression: {'ON' if self.compress_var.get() else 'OFF'}")
        self.log(f"{'='*60}\n")
        
        self.progress_bar['maximum'] = self.total_sprites
        self.progress_bar['value'] = 0
        
        success_count = 0
        fail_count = 0
        
        for i, png_file in enumerate(sorted(png_files), 1):
            input_path = os.path.join(input_folder, png_file)
            sprite_name = os.path.splitext(png_file)[0]
            resized_path = os.path.join(self.temp_dir, png_file)
            
            # Progress
            if i % 100 == 0:
                self.log(f"Processing... {i}/{self.total_sprites}")
            
            # Step 1: Resize PNG
            if not self.resize_png(input_path, resized_path, target_size):
                self.log(f"❌ {png_file}: Resize failed")
                fail_count += 1
                continue
            
            # Step 2: Convert with grit (using resized PNG)
            success, message = self.convert_with_grit(resized_path, output_folder, sprite_name)
            
            if success:
                success_count += 1
            else:
                fail_count += 1
                self.log(f"❌ {png_file}: {message}")
            
            self.sprites_converted += 1
            self.progress_bar['value'] = self.sprites_converted
            self.progress_label.config(text=f"Converted: {self.sprites_converted}/{self.total_sprites}")
            self.root.update()
        
        # Cleanup temp folder
        try:
            shutil.rmtree(self.temp_dir)
            self.log(f"Cleaned up temp folder")
        except:
            pass
        
        # Summary
        self.log(f"\n{'='*60}")
        self.log(f"Conversion complete!")
        self.log(f"✓ Success: {success_count}")
        if fail_count > 0:
            self.log(f"❌ Failed: {fail_count}")
        self.log(f"{'='*60}\n")
        
        # Verify output size
        if success_count > 0:
            first_file = os.path.join(output_folder, os.path.splitext(sorted(png_files)[0])[0] + '.c')
            if os.path.exists(first_file):
                with open(first_file, 'r') as f:
                    lines = f.read(500)
                    if 'x' in lines:
                        import re
                        size_match = re.search(r'(\d+)x(\d+)', lines)
                        if size_match:
                            actual_size = f"{size_match.group(1)}x{size_match.group(2)}"
                            self.log(f"✓ Verified output: {actual_size}")
                            
                            # Check file size
                            size_bytes = os.path.getsize(first_file)
                            size_kb = size_bytes / 1024
                            self.log(f"✓ File size: {size_kb:.1f} KB (should be ~4-5 KB for 64x64)")
        
        messagebox.showinfo("Complete", 
                          f"Converted {success_count} sprites!\n"
                          f"Resized from source → {size_str}\n"
                          f"Check log for details")

if __name__ == "__main__":
    root = tk.Tk()
    app = SpriteConverter(root)
    root.mainloop()
