#!/usr/bin/env python3
"""
Pokemon Database Converter - Python to C (GBA)
Converts pokemon_database.py to GBA C format with full form support
Handles: Base forms, single letter forms (681a), double letter forms (869aa, 869ab...)
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import os
import re

class PokemonDatabaseConverter:
    def __init__(self, root):
        self.root = root
        self.root.title("Pokemon Database Converter - Python to GBA C")
        self.root.geometry("1000x700")
        
        self.pokemon_data = None
        self.create_widgets()
    
    def create_widgets(self):
        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # File selection
        ttk.Label(main_frame, text="Python Database File:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.file_entry = ttk.Entry(main_frame, width=60)
        self.file_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), pady=5)
        ttk.Button(main_frame, text="Browse", command=self.browse_file).grid(row=0, column=2, padx=5, pady=5)
        
        # Pokemon count
        ttk.Label(main_frame, text="Total Pokemon:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.count_label = ttk.Label(main_frame, text="0", font=('Arial', 10, 'bold'))
        self.count_label.grid(row=1, column=1, sticky=tk.W, pady=5)
        
        # Limit selection
        ttk.Label(main_frame, text="Limit to (0 = all):").grid(row=2, column=0, sticky=tk.W, pady=5)
        self.limit_var = tk.IntVar(value=0)
        ttk.Spinbox(main_frame, from_=0, to=1000, textvariable=self.limit_var, width=10).grid(row=2, column=1, sticky=tk.W, pady=5)
        
        # Convert button
        ttk.Button(main_frame, text="Convert to C", command=self.convert, 
                  style='Accent.TButton').grid(row=3, column=0, columnspan=3, pady=10)
        
        # Progress
        self.progress = ttk.Progressbar(main_frame, mode='determinate')
        self.progress.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)
        
        # Status
        self.status_label = ttk.Label(main_frame, text="Ready", foreground="green")
        self.status_label.grid(row=5, column=0, columnspan=3, pady=5)
        
        # Preview
        ttk.Label(main_frame, text="Preview:").grid(row=6, column=0, sticky=tk.W, pady=5)
        self.preview_text = scrolledtext.ScrolledText(main_frame, height=25, width=120, font=('Courier', 9))
        self.preview_text.grid(row=7, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        # Save button
        ttk.Button(main_frame, text="Save to File", command=self.save_file).grid(row=8, column=0, columnspan=3, pady=10)
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(7, weight=1)
    
    def browse_file(self):
        filename = filedialog.askopenfilename(
            title="Select Pokemon Database",
            filetypes=[("Python files", "*.py"), ("All files", "*.*")]
        )
        if filename:
            self.file_entry.delete(0, tk.END)
            self.file_entry.insert(0, filename)
            self.load_database(filename)
    
    def load_database(self, filepath):
        try:
            self.status_label.config(text="Loading database...", foreground="orange")
            self.root.update()
            
            # Read and execute Python file
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            namespace = {}
            exec(content, namespace)
            
            self.pokemon_data = namespace.get('COMPLETE_POKEMON_POOL', {})
            
            if self.pokemon_data:
                count = len(self.pokemon_data)
                self.count_label.config(text=str(count))
                self.status_label.config(text=f"Loaded {count} Pokemon successfully!", foreground="green")
            else:
                raise ValueError("No COMPLETE_POKEMON_POOL found")
                
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load database:\n{str(e)}")
            self.status_label.config(text="Error loading database", foreground="red")
    
    def extract_dex_number(self, pokedex_num):
        """Extract numeric part and form suffix from pokedex number"""
        if isinstance(pokedex_num, int):
            return pokedex_num, ""
        
        # String like "681a" or "869aa"
        pokedex_str = str(pokedex_num)
        match = re.match(r'(\d+)([a-z]*)', pokedex_str)
        if match:
            num = int(match.group(1))
            suffix = match.group(2)
            return num, suffix
        
        return int(pokedex_str), ""
    
    def get_sprite_filename(self, pokedex_num):
        """Generate sprite filename: 0681.png or 0681a.png or 0869aa.png"""
        num, suffix = self.extract_dex_number(pokedex_num)
        return f"{num:04d}{suffix}.png"
    
    def sanitize_c_string(self, text):
        """Make text safe for C strings"""
        if text is None:
            return "NULL"
        
        text = str(text)
        # Escape quotes and backslashes
        text = text.replace('\\', '\\\\')
        text = text.replace('"', '\\"')
        # Remove newlines
        text = text.replace('\n', ' ').replace('\r', '')
        # Remove problematic Unicode characters
        text = text.encode('ascii', 'ignore').decode('ascii')
        
        return f'"{text}"'
    
    def get_mode_bitmask(self, modes):
        """Convert mode list to C bitmask expression"""
        mode_map = {
            'rookie': 'MODE_ROOKIE',
            'normal': 'MODE_NORMAL',
            'super': 'MODE_SUPER',
            'hyper': 'MODE_HYPER',
            'master': 'MODE_MASTER',
            'alcremie': 'MODE_ALCREMIE',  # Bonus mode
            'vivillon': 'MODE_VIVILLON',  # Bonus mode
            'unown': 'MODE_UNOWN'  # Bonus mode
        }
        
        if not modes:
            return "0"
        
        bitmask_parts = []
        for mode in modes:
            if mode in mode_map:
                bitmask_parts.append(mode_map[mode])
        
        return " | ".join(bitmask_parts) if bitmask_parts else "0"
    
    def get_turn_values(self, turns):
        """Get turn counts for all modes [rookie, normal, super, hyper, master, unown, vivillon, alcremie]"""
        return [
            turns.get('rookie', 0),
            turns.get('normal', 0),
            turns.get('super', 0),
            turns.get('hyper', 0),
            turns.get('master', 0),
            turns.get('unown', 0),
            turns.get('vivillon', 0),
            turns.get('alcremie', 0)
        ]
    
    def capitalize_pokemon_name(self, name):
        """Properly capitalize Pokemon names"""
        # Special cases
        special_names = {
            'farfetchd': "Farfetch'd",
            'mr. mime': 'Mr. Mime',
            'mr mime': 'Mr. Mime',
            'mime jr.': 'Mime Jr.',
            'mime jr': 'Mime Jr.',
            'nidoran-f': 'Nidoran♀',
            'nidoran-m': 'Nidoran♂',
            'ho-oh': 'Ho-Oh',
            'porygon-z': 'Porygon-Z',
            'type: null': 'Type: Null',
            'jangmo-o': 'Jangmo-o',
            'hakamo-o': 'Hakamo-o',
            'kommo-o': 'Kommo-o',
            'tapu koko': 'Tapu Koko',
            'tapu lele': 'Tapu Lele',
            'tapu bulu': 'Tapu Bulu',
            'tapu fini': 'Tapu Fini'
        }
        
        name_lower = name.lower()
        if name_lower in special_names:
            return special_names[name_lower]
        
        # Handle forms in parentheses
        if '(' in name:
            # Split into base and form
            parts = name.split('(')
            base = parts[0].strip().title()
            form = '(' + '('.join(parts[1:])
            form = form.title()
            return base + ' ' + form
        
        return name.title()
    
    def convert_pokemon_entry(self, name, data):
        """Convert single Pokemon to C format"""
        # Extract data
        dex_num_raw = data.get('pokedex_num', 0)
        dex_num, form_suffix = self.extract_dex_number(dex_num_raw)
        
        modes = data.get('modes', [])
        turns = data.get('turns', {})
        special = 1 if data.get('special', False) else 0
        
        type1 = self.sanitize_c_string(data.get('type1', 'Normal'))
        type2 = self.sanitize_c_string(data.get('type2'))
        height = self.sanitize_c_string(data.get('height', '0.0 m'))
        weight = self.sanitize_c_string(data.get('weight', '0.0 kg'))
        dex_text = self.sanitize_c_string(data.get('dex_text', 'No data available.'))
        
        # Format values
        display_name = self.capitalize_pokemon_name(name)
        mode_bitmask = self.get_mode_bitmask(modes)
        turn_values = self.get_turn_values(turns)
        sprite_file = self.get_sprite_filename(dex_num_raw)
        
        # Build C entry with 8 turn values (standard 5 + bonus 3)
        # Line 1: dex number, name, modes
        # Line 2: turn values (r, n, s, h, m, un, viv, alc), special, sprite, types
        # Line 3: height, weight, dex_text
        c_entry = f"""    {{{dex_num}, "{display_name}", {mode_bitmask},
     {turn_values[0]}, {turn_values[1]}, {turn_values[2]}, {turn_values[3]}, {turn_values[4]}, {turn_values[5]}, {turn_values[6]}, {turn_values[7]}, {special}, "{sprite_file}", {type1}, {type2},
     {height}, {weight}, {dex_text}}},
"""
        return c_entry
    
    def convert(self):
        if not self.pokemon_data:
            messagebox.showwarning("Warning", "Please load a database first!")
            return
        
        try:
            self.status_label.config(text="Converting...", foreground="orange")
            self.root.update()
            
            # Sort by dex number (numeric part)
            sorted_pokemon = sorted(
                self.pokemon_data.items(),
                key=lambda x: (
                    self.extract_dex_number(x[1].get('pokedex_num', 0))[0],  # Numeric part
                    self.extract_dex_number(x[1].get('pokedex_num', 0))[1]   # Form suffix
                )
            )
            
            # Apply limit
            limit = self.limit_var.get()
            if limit > 0:
                sorted_pokemon = sorted_pokemon[:limit]
            
            total = len(sorted_pokemon)
            
            # Build C code
            c_code = f"""#include "pokemon_database.h"

// Pokemon database - Converted from Python
// Total Pokemon: {total}
// Format: {{dex_num, "name", modes, r, n, s, h, m, un, viv, alc, special, "sprite", "type1", "type2", "height", "weight", "dex_text"}}
// Where: r=rookie, n=normal, s=super, h=hyper, m=master, un=unown, viv=vivillon, alc=alcremie

const PokemonData POKEMON_DATABASE[TOTAL_POKEMON] = {{
"""
            
            # Convert each Pokemon
            self.progress['maximum'] = total
            for i, (name, data) in enumerate(sorted_pokemon):
                c_code += self.convert_pokemon_entry(name, data)
                self.progress['value'] = i + 1
                self.root.update_idletasks()
            
            c_code += """}};

// Helper functions
const PokemonData* get_pokemon_data(int index) {
    if(index >= 0 && index < TOTAL_POKEMON) {
        return &POKEMON_DATABASE[index];
    }
    return &POKEMON_DATABASE[0];
}

u8 get_pokemon_turns(int index, u8 mode) {
    const PokemonData* pdata = get_pokemon_data(index);
    switch(mode) {
        case 0: return pdata->turns_rookie;   // MODE_ROOKIE
        case 1: return pdata->turns_normal;   // MODE_NORMAL
        case 2: return pdata->turns_super;    // MODE_SUPER
        case 3: return pdata->turns_hyper;    // MODE_HYPER
        case 4: return pdata->turns_master;   // MODE_MASTER
        case 5: return pdata->turns_unown;    // MODE_UNOWN (Bonus)
        case 6: return pdata->turns_vivillon; // MODE_VIVILLON (Bonus)
        case 7: return pdata->turns_alcremie; // MODE_ALCREMIE (Bonus)
        default: return 11;  // Fallback
    }
}
"""
            
            # Update header file count
            header_update = f"""// Update this in pokemon_database.h:
#define TOTAL_POKEMON {total}
"""
            
            # Show in preview
            self.preview_text.delete(1.0, tk.END)
            self.preview_text.insert(1.0, header_update + "\n" + c_code)
            
            self.c_code = c_code
            self.header_update = header_update
            
            self.status_label.config(text=f"Converted {total} Pokemon successfully!", foreground="green")
            self.progress['value'] = 0
            
        except Exception as e:
            messagebox.showerror("Error", f"Conversion failed:\n{str(e)}")
            self.status_label.config(text="Conversion failed", foreground="red")
    
    def save_file(self):
        if not hasattr(self, 'c_code'):
            messagebox.showwarning("Warning", "Please convert the database first!")
            return
        
        filename = filedialog.asksaveasfilename(
            title="Save C Database",
            defaultextension=".c",
            filetypes=[("C source", "*.c"), ("All files", "*.*")],
            initialfile="pokemon_database.c"
        )
        
        if filename:
            try:
                with open(filename, 'w', encoding='utf-8') as f:
                    f.write(self.c_code)
                
                # Also save header note
                note_file = filename.replace('.c', '_HEADER_UPDATE.txt')
                with open(note_file, 'w', encoding='utf-8') as f:
                    f.write(self.header_update)
                
                messagebox.showinfo("Success", 
                    f"Saved successfully!\n\n"
                    f"Database: {filename}\n"
                    f"Header update: {note_file}\n\n"
                    f"Remember to update TOTAL_POKEMON in pokemon_database.h!")
                
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save:\n{str(e)}")

def main():
    root = tk.Tk()
    app = PokemonDatabaseConverter(root)
    root.mainloop()

if __name__ == "__main__":
    main()
