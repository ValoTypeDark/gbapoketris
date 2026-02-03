#!/usr/bin/env python3
"""
GBA Font Converter - GUI Version
Convert TrueType/OpenType fonts to GBA bitmap format with live preview
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, font as tkfont
from PIL import Image, ImageDraw, ImageFont, ImageTk
import os
import sys

class FontConverterGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("GBA Font Converter")
        self.root.geometry("800x700")
        self.root.resizable(True, True)
        
        self.font_path = None
        self.preview_image = None
        
        self.setup_ui()
        
    def setup_ui(self):
        # Main container
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(5, weight=1)
        
        # Title
        title_label = ttk.Label(main_frame, text="GBA Font Converter", 
                               font=('Arial', 16, 'bold'))
        title_label.grid(row=0, column=0, columnspan=3, pady=(0, 20))
        
        # Font Selection
        font_frame = ttk.LabelFrame(main_frame, text="Font Selection", padding="10")
        font_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))
        font_frame.columnconfigure(1, weight=1)
        
        ttk.Label(font_frame, text="Font File:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.font_path_var = tk.StringVar()
        font_entry = ttk.Entry(font_frame, textvariable=self.font_path_var, state='readonly')
        font_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), padx=5)
        
        browse_btn = ttk.Button(font_frame, text="Browse...", command=self.browse_font)
        browse_btn.grid(row=0, column=2, padx=(5, 0))
        
        # Quick select system fonts OR dropdown list
        quick_frame = ttk.Frame(font_frame)
        quick_frame.grid(row=1, column=0, columnspan=3, pady=(10, 0))
        
        ttk.Label(quick_frame, text="Quick Select:").pack(side=tk.LEFT, padx=(0, 5))
        
        # For Windows, add a dropdown of all fonts
        if sys.platform == 'win32':
            self.setup_windows_font_list(quick_frame)
        else:
            # For Mac/Linux, use buttons
            system_fonts = self.get_system_fonts()
            for font_name, font_file in system_fonts:
                btn = ttk.Button(quick_frame, text=font_name, 
                               command=lambda f=font_file: self.load_font(f))
                btn.pack(side=tk.LEFT, padx=2)
        
        # Settings
        settings_frame = ttk.LabelFrame(main_frame, text="Settings", padding="10")
        settings_frame.grid(row=2, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(settings_frame, text="Font Size (pt):").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.size_var = tk.IntVar(value=8)
        size_spinbox = ttk.Spinbox(settings_frame, from_=6, to=20, textvariable=self.size_var,
                                   command=self.update_preview, width=10)
        size_spinbox.grid(row=0, column=1, sticky=tk.W, padx=5)
        
        ttk.Label(settings_frame, text="Output Name:").grid(row=0, column=2, sticky=tk.W, padx=(20, 5))
        self.output_name_var = tk.StringVar(value="custom_font")
        output_entry = ttk.Entry(settings_frame, textvariable=self.output_name_var, width=20)
        output_entry.grid(row=0, column=3, sticky=tk.W, padx=5)
        
        # Character set selection
        ttk.Label(settings_frame, text="Character Set:").grid(row=1, column=0, sticky=tk.W, padx=(0, 5), pady=(10, 0))
        self.charset_var = tk.StringVar(value="uppercase")
        
        charset_frame = ttk.Frame(settings_frame)
        charset_frame.grid(row=1, column=1, columnspan=3, sticky=tk.W, pady=(10, 0))
        
        ttk.Radiobutton(charset_frame, text="Uppercase Only (32-90)", 
                       variable=self.charset_var, value="uppercase",
                       command=self.update_preview).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(charset_frame, text="All Printable (32-126)", 
                       variable=self.charset_var, value="all",
                       command=self.update_preview).pack(side=tk.LEFT, padx=5)
        
        # Preview Update Button
        preview_btn = ttk.Button(settings_frame, text="Update Preview", 
                               command=self.update_preview)
        preview_btn.grid(row=2, column=0, columnspan=4, pady=(10, 0))
        
        # Preview
        preview_frame = ttk.LabelFrame(main_frame, text="Preview", padding="10")
        preview_frame.grid(row=3, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        preview_frame.columnconfigure(0, weight=1)
        preview_frame.rowconfigure(0, weight=1)
        
        # Canvas for preview with scrollbar
        canvas_frame = ttk.Frame(preview_frame)
        canvas_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        canvas_frame.columnconfigure(0, weight=1)
        canvas_frame.rowconfigure(0, weight=1)
        
        self.preview_canvas = tk.Canvas(canvas_frame, bg='white', height=200)
        scrollbar = ttk.Scrollbar(canvas_frame, orient=tk.VERTICAL, command=self.preview_canvas.yview)
        self.preview_canvas.configure(yscrollcommand=scrollbar.set)
        
        self.preview_canvas.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        
        # Info label
        self.info_label = ttk.Label(preview_frame, text="Select a font to see preview")
        self.info_label.grid(row=1, column=0, pady=(10, 0))
        
        # Output
        output_frame = ttk.LabelFrame(main_frame, text="Output", padding="10")
        output_frame.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))
        output_frame.columnconfigure(1, weight=1)
        
        ttk.Label(output_frame, text="Save To:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.output_dir_var = tk.StringVar(value=os.getcwd())
        output_entry = ttk.Entry(output_frame, textvariable=self.output_dir_var, state='readonly')
        output_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), padx=5)
        
        browse_output_btn = ttk.Button(output_frame, text="Browse...", 
                                       command=self.browse_output)
        browse_output_btn.grid(row=0, column=2, padx=(5, 0))
        
        # Status
        self.status_var = tk.StringVar(value="Ready")
        status_label = ttk.Label(main_frame, textvariable=self.status_var, 
                                relief=tk.SUNKEN, anchor=tk.W)
        status_label.grid(row=5, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.S), pady=(0, 0))
        
        # Convert Button
        convert_btn = ttk.Button(main_frame, text="Convert to GBA Format", 
                               command=self.convert_font, style='Accent.TButton')
        convert_btn.grid(row=5, column=2, sticky=(tk.E, tk.S), pady=(0, 0))
        
    def setup_windows_font_list(self, parent):
        """Set up Windows font dropdown list with search"""
        fonts = self.scan_windows_fonts()
        
        if fonts:
            # Store all fonts
            self.all_fonts = fonts
            self.windows_fonts = {name: path for name, path in fonts}
            
            # Search box
            search_frame = ttk.Frame(parent)
            search_frame.pack(side=tk.LEFT, padx=5)
            
            ttk.Label(search_frame, text="Search:").pack(side=tk.LEFT)
            self.font_search_var = tk.StringVar()
            self.font_search_var.trace('w', self.filter_fonts)
            search_entry = ttk.Entry(search_frame, textvariable=self.font_search_var, width=20)
            search_entry.pack(side=tk.LEFT, padx=(5, 0))
            
            # Font dropdown
            self.selected_font_var = tk.StringVar()
            self.font_combo = ttk.Combobox(parent, textvariable=self.selected_font_var, 
                                     values=[f[0] for f in fonts], width=40, state='readonly')
            self.font_combo.pack(side=tk.LEFT, padx=5)
            self.font_combo.bind('<<ComboboxSelected>>', self.on_font_selected)
            
            # Show count
            self.font_count_label = ttk.Label(parent, text=f"({len(fonts)} fonts)")
            self.font_count_label.pack(side=tk.LEFT, padx=5)
            
            if fonts:
                self.font_combo.current(0)
        else:
            ttk.Label(parent, text="No fonts found").pack(side=tk.LEFT, padx=5)
    
    def filter_fonts(self, *args):
        """Filter font list based on search"""
        search_term = self.font_search_var.get().lower()
        
        if search_term:
            filtered = [f for f in self.all_fonts if search_term in f[0].lower()]
        else:
            filtered = self.all_fonts
        
        self.font_combo['values'] = [f[0] for f in filtered]
        self.font_count_label.config(text=f"({len(filtered)} fonts)")
        
        if filtered:
            self.font_combo.current(0)
            self.selected_font_var.set(filtered[0][0])
    
    def on_font_selected(self, event):
        """Handle font selection from dropdown"""
        font_name = self.selected_font_var.get()
        if font_name in self.windows_fonts:
            self.load_font(self.windows_fonts[font_name])
    
    def scan_windows_fonts(self):
        """Scan Windows Fonts using Registry and user fonts folder"""
        import winreg
        import os
        
        fonts = []
        system_font_dir = 'C:\\Windows\\Fonts\\'
        user_font_dir = os.path.join(os.environ.get('LOCALAPPDATA', ''), 'Microsoft\\Windows\\Fonts\\')
        
        # Scan system fonts from Registry
        try:
            registry_key = winreg.OpenKey(
                winreg.HKEY_LOCAL_MACHINE,
                r"SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts",
                0,
                winreg.KEY_READ
            )
            
            i = 0
            while True:
                try:
                    font_name, font_file, _ = winreg.EnumValue(registry_key, i)
                    i += 1
                    
                    # Build full path
                    if not os.path.isabs(font_file):
                        font_file = os.path.join(system_font_dir, font_file)
                    
                    # Only include if file exists and is a font file
                    if os.path.exists(font_file) and font_file.lower().endswith(('.ttf', '.otf', '.ttc')):
                        # Clean up the display name
                        display_name = font_name.replace(' (TrueType)', '').replace(' (OpenType)', '')
                        fonts.append((display_name, font_file))
                        
                except OSError:
                    break
            
            winreg.CloseKey(registry_key)
        except Exception as e:
            print(f"System fonts registry scan failed: {e}")
        
        # Scan user fonts from Registry
        try:
            registry_key = winreg.OpenKey(
                winreg.HKEY_CURRENT_USER,
                r"SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts",
                0,
                winreg.KEY_READ
            )
            
            i = 0
            while True:
                try:
                    font_name, font_file, _ = winreg.EnumValue(registry_key, i)
                    i += 1
                    
                    # Build full path
                    if not os.path.isabs(font_file):
                        font_file = os.path.join(user_font_dir, font_file)
                    
                    # Only include if file exists and is a font file
                    if os.path.exists(font_file) and font_file.lower().endswith(('.ttf', '.otf', '.ttc')):
                        # Clean up the display name
                        display_name = font_name.replace(' (TrueType)', '').replace(' (OpenType)', '')
                        fonts.append((f"{display_name} [User]", font_file))
                        
                except OSError:
                    break
            
            winreg.CloseKey(registry_key)
        except Exception as e:
            print(f"User fonts registry scan failed: {e}")
        
        # Also scan user fonts folder directly (in case not in registry)
        if os.path.exists(user_font_dir):
            import glob
            for font_file in glob.glob(os.path.join(user_font_dir, '*.ttf')):
                font_name = os.path.basename(font_file)
                # Check if already added from registry
                if not any(f[1] == font_file for f in fonts):
                    fonts.append((f"{font_name} [User]", font_file))
            for font_file in glob.glob(os.path.join(user_font_dir, '*.otf')):
                font_name = os.path.basename(font_file)
                if not any(f[1] == font_file for f in fonts):
                    fonts.append((f"{font_name} [User]", font_file))
        
        # Fallback to file system scan if registry failed
        if not fonts:
            fonts = self.scan_fonts_filesystem()
        
        # Sort alphabetically
        fonts.sort(key=lambda x: x[0].lower())
        
        return fonts
    
    def scan_fonts_filesystem(self):
        """Fallback: Scan file system for fonts"""
        import glob
        
        fonts = []
        font_dir = 'C:\\Windows\\Fonts\\'
        
        if not os.path.exists(font_dir):
            return fonts
        
        # Scan for all font file types (including subfolders)
        font_extensions = ['*.ttf', '*.otf', '*.ttc', '*.TTF', '*.OTF', '*.TTC']
        
        for ext in font_extensions:
            # Search in main directory
            for font_file in glob.glob(os.path.join(font_dir, ext)):
                font_name = os.path.basename(font_file)
                fonts.append((font_name, font_file))
            
            # Search in subdirectories
            for font_file in glob.glob(os.path.join(font_dir, '**', ext), recursive=True):
                font_name = os.path.basename(font_file)
                fonts.append((font_name, font_file))
        
        # Remove duplicates
        fonts = list(set(fonts))
        fonts.sort(key=lambda x: x[0].lower())
        
        return fonts
    
    def get_system_fonts(self):
        """Get common system fonts"""
        fonts = []
        
        if sys.platform == 'win32':
            font_dir = 'C:\\Windows\\Fonts\\'
            common = [
                ('Arial', 'arial.ttf'),
                ('Comic Sans', 'comic.ttf'),
                ('Courier', 'cour.ttf'),
                ('Verdana', 'verdana.ttf'),
            ]
        elif sys.platform == 'darwin':
            font_dir = '/System/Library/Fonts/'
            common = [
                ('Arial', 'Arial.ttf'),
                ('Courier', 'Courier.ttc'),
                ('Helvetica', 'Helvetica.ttc'),
            ]
        else:  # Linux
            font_dir = '/usr/share/fonts/truetype/'
            common = [
                ('Liberation', 'liberation/LiberationSans-Regular.ttf'),
                ('DejaVu', 'dejavu/DejaVuSans.ttf'),
            ]
        
        for name, filename in common:
            full_path = os.path.join(font_dir, filename)
            if os.path.exists(full_path):
                fonts.append((name, full_path))
        
        return fonts[:4]  # Limit to 4 quick buttons
    
    def browse_font(self):
        """Browse for font file"""
        # Determine initial directory based on OS
        if sys.platform == 'win32':
            initial_dir = 'C:\\Windows\\Fonts'
        elif sys.platform == 'darwin':
            initial_dir = '/System/Library/Fonts'
        else:
            initial_dir = '/usr/share/fonts/truetype'
        
        # Check if initial directory exists, otherwise use home
        if not os.path.exists(initial_dir):
            initial_dir = os.path.expanduser('~')
        
        filename = filedialog.askopenfilename(
            title="Select Font File",
            initialdir=initial_dir,
            filetypes=[
                ("Font Files", "*.ttf;*.otf;*.ttc"),
                ("TrueType Font", "*.ttf"),
                ("OpenType Font", "*.otf"),
                ("TrueType Collection", "*.ttc"),
                ("All Files", "*.*")
            ]
        )
        
        if filename:
            self.load_font(filename)
    
    def load_font(self, font_path):
        """Load a font and update preview"""
        self.font_path = font_path
        self.font_path_var.set(font_path)
        
        # Auto-generate output name from font filename
        base_name = os.path.splitext(os.path.basename(font_path))[0]
        base_name = base_name.lower().replace(' ', '_').replace('-', '_')
        self.output_name_var.set(f"{base_name}{self.size_var.get()}")
        
        self.update_preview()
        self.status_var.set(f"Loaded: {os.path.basename(font_path)}")
    
    def browse_output(self):
        """Browse for output directory"""
        directory = filedialog.askdirectory(title="Select Output Directory")
        if directory:
            self.output_dir_var.set(directory)
    
    def get_charset(self):
        """Get character set based on selection"""
        if self.charset_var.get() == "uppercase":
            return ''.join(chr(i) for i in range(32, 91))
        else:
            return ''.join(chr(i) for i in range(32, 127))
    
    def update_preview(self):
        """Update the preview canvas"""
        if not self.font_path:
            return
        
        try:
            font_size = self.size_var.get()
            font = ImageFont.truetype(self.font_path, font_size)
            
            chars = self.get_charset()
            
            # Create preview image
            preview_width = 760
            chars_per_row = 20
            rows = (len(chars) + chars_per_row - 1) // chars_per_row
            
            # Calculate dimensions
            char_spacing = 4
            max_char_width = font_size + char_spacing
            row_height = font_size + 10
            
            preview_height = rows * row_height + 20
            
            img = Image.new('RGB', (preview_width, preview_height), 'white')
            draw = ImageDraw.Draw(img)
            
            # Draw grid and characters
            x, y = 10, 10
            max_width = 0
            max_height = 0
            
            for i, char in enumerate(chars):
                # Draw character
                try:
                    draw.text((x, y), char, font=font, fill='black')
                    
                    # Calculate actual dimensions
                    bbox = font.getbbox(char)
                    if bbox:
                        w = bbox[2] - bbox[0]
                        h = bbox[3] - bbox[1]
                        max_width = max(max_width, w)
                        max_height = max(max_height, h)
                except:
                    pass
                
                x += max_char_width
                
                if (i + 1) % chars_per_row == 0:
                    x = 10
                    y += row_height
            
            # Update info
            self.info_label.config(
                text=f"Character size: ~{max_width}×{max_height} pixels | "
                     f"Total characters: {len(chars)} | "
                     f"Estimated size: ~{len(chars) * max_height} bytes"
            )
            
            # Convert to PhotoImage and display
            self.preview_image = ImageTk.PhotoImage(img)
            self.preview_canvas.delete("all")
            self.preview_canvas.create_image(0, 0, anchor=tk.NW, image=self.preview_image)
            self.preview_canvas.config(scrollregion=(0, 0, preview_width, preview_height))
            
        except Exception as e:
            messagebox.showerror("Preview Error", f"Error generating preview:\n{str(e)}")
    
    def convert_font(self):
        """Convert font to GBA format"""
        if not self.font_path:
            messagebox.showwarning("No Font", "Please select a font file first.")
            return
        
        output_name = self.output_name_var.get().strip()
        if not output_name:
            messagebox.showwarning("No Output Name", "Please enter an output name.")
            return
        
        # Sanitize output name
        output_name = ''.join(c if c.isalnum() or c == '_' else '_' for c in output_name)
        
        try:
            self.status_var.set("Converting...")
            self.root.update()
            
            font_size = self.size_var.get()
            output_dir = self.output_dir_var.get()
            chars = self.get_charset()
            
            # Generate font
            success = self.generate_font_bitmap(
                self.font_path, 
                font_size, 
                output_name, 
                output_dir,
                chars
            )
            
            if success:
                self.status_var.set("Conversion complete!")
                result = messagebox.showinfo(
                    "Success", 
                    f"Font converted successfully!\n\n"
                    f"Generated files:\n"
                    f"  • font_{output_name}.c\n"
                    f"  • font_{output_name}.h\n\n"
                    f"Location: {output_dir}\n\n"
                    f"To use in your GBA project:\n"
                    f"1. Copy both files to your project\n"
                    f"2. #include \"font_{output_name}.h\"\n"
                    f"3. Use draw_char_{output_name}() function"
                )
            else:
                self.status_var.set("Conversion failed")
                
        except Exception as e:
            self.status_var.set("Error during conversion")
            messagebox.showerror("Conversion Error", f"Error converting font:\n{str(e)}")
    
    def generate_font_bitmap(self, font_path, font_size, output_name, output_dir, chars):
        """Generate bitmap font data from a TrueType font"""
        
        try:
            font = ImageFont.truetype(font_path, font_size)
        except Exception as e:
            messagebox.showerror("Font Error", f"Error loading font:\n{str(e)}")
            return False
        
        # Determine character dimensions
        max_width = 0
        max_height = 0
        char_data = {}
        
        for char in chars:
            temp_img = Image.new('1', (32, 32), 0)
            draw = ImageDraw.Draw(temp_img)
            draw.text((0, 0), char, font=font, fill=1)
            
            bbox = temp_img.getbbox()
            if bbox:
                width = bbox[2] - bbox[0]
                height = bbox[3] - bbox[1]
                max_width = max(max_width, width)
                max_height = max(max_height, height)
                char_data[char] = (temp_img, bbox)
        
        # Generate C code
        c_code = []
        c_code.append(f"// Generated font data from {os.path.basename(font_path)}")
        c_code.append(f"// Font size: {font_size}pt, Character size: {max_width}x{max_height}")
        c_code.append(f"// Generated by GBA Font Converter")
        c_code.append("")
        c_code.append("#include <gba_types.h>")
        c_code.append("")
        c_code.append(f"#define FONT_{output_name.upper()}_WIDTH {max_width}")
        c_code.append(f"#define FONT_{output_name.upper()}_HEIGHT {max_height}")
        c_code.append("")
        c_code.append(f"const u8 font_{output_name}_data[][{max_height}] = {{")
        
        for i, char in enumerate(chars):
            if char not in char_data:
                c_code.append(f"    {{{', '.join(['0x00'] * max_height)}}}, // {repr(char)}")
                continue
            
            img, bbox = char_data[char]
            bitmap_rows = []
            
            for y in range(max_height):
                row_value = 0
                for x in range(max_width):
                    px_x = bbox[0] + x if bbox else x
                    px_y = bbox[1] + y if bbox else y
                    
                    if px_x < img.width and px_y < img.height:
                        pixel = img.getpixel((px_x, px_y))
                        if pixel:
                            row_value |= (1 << (max_width - 1 - x))
                
                bitmap_rows.append(f"0x{row_value:02X}")
            
            c_code.append(f"    {{{', '.join(bitmap_rows)}}}, // '{char}' ({ord(char)})")
        
        c_code.append("};")
        
        # Write C file
        c_file = os.path.join(output_dir, f"font_{output_name}.c")
        with open(c_file, 'w') as f:
            f.write('\n'.join(c_code))
        
        # Generate header
        h_code = []
        h_code.append(f"#ifndef FONT_{output_name.upper()}_H")
        h_code.append(f"#define FONT_{output_name.upper()}_H")
        h_code.append("")
        h_code.append("#include <gba_types.h>")
        h_code.append("")
        h_code.append(f"#define FONT_{output_name.upper()}_WIDTH {max_width}")
        h_code.append(f"#define FONT_{output_name.upper()}_HEIGHT {max_height}")
        h_code.append("")
        h_code.append(f"extern const u8 font_{output_name}_data[][{max_height}];")
        h_code.append("")
        h_code.append(f"// Helper function to draw a character")
        h_code.append(f"static inline void draw_char_{output_name}(int x, int y, char c, u16 color, u16* buffer, int buffer_width) {{")
        h_code.append(f"    if(c < 32 || c >= {32 + len(chars)}) c = 32;")
        h_code.append(f"    const u8* glyph = font_{output_name}_data[c - 32];")
        h_code.append(f"    for(int row = 0; row < FONT_{output_name.upper()}_HEIGHT; row++) {{")
        h_code.append(f"        u8 row_data = glyph[row];")
        h_code.append(f"        for(int col = 0; col < FONT_{output_name.upper()}_WIDTH; col++) {{")
        h_code.append(f"            if(row_data & (1 << (FONT_{output_name.upper()}_WIDTH - 1 - col))) {{")
        h_code.append(f"                int px = x + col;")
        h_code.append(f"                int py = y + row;")
        h_code.append(f"                if(px >= 0 && px < buffer_width && py >= 0 && py < 160) {{")
        h_code.append(f"                    buffer[py * buffer_width + px] = color;")
        h_code.append(f"                }}")
        h_code.append(f"            }}")
        h_code.append(f"        }}")
        h_code.append(f"    }}")
        h_code.append(f"}}")
        h_code.append("")
        h_code.append("#endif")
        
        # Write header file
        h_file = os.path.join(output_dir, f"font_{output_name}.h")
        with open(h_file, 'w') as f:
            f.write('\n'.join(h_code))
        
        return True

def main():
    root = tk.Tk()
    app = FontConverterGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
