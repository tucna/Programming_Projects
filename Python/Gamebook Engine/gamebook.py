import re
import os

def load_gamebook(filename):
    """Load a gamebook from a file and return a dictionary of sections."""
    sections = {}
    
    try:
        with open(filename, 'r', encoding='utf-8') as file:
            lines = file.readlines()
        
        current_section = None
        current_content = []
        
        for line in lines:
            line = line.rstrip()
            
            # Check if this is a section marker line
            if (line.startswith('#') and len(line) > 1 and line[1:].isdigit()) or \
               (line.endswith('#') and len(line) > 1 and line[:-1].isdigit()):
                
                # Extract section ID
                if line.startswith('#'):
                    section_id = line[1:]
                else:  # line.endswith('#')
                    section_id = line[:-1]
                
                # Save previous section if exists
                if current_section is not None:
                    sections[current_section] = '\n'.join(current_content).strip()
                
                # Start new section
                current_section = section_id
                current_content = []
            elif current_section is not None:
                current_content.append(line)
        
        # Save the last section
        if current_section is not None:
            sections[current_section] = '\n'.join(current_content).strip()
        
        return sections
    except Exception as e:
        print(f"Error loading gamebook: {e}")
        return {}

def find_choices(text):
    """Find all choices in a section text and return them as a list of section IDs."""
    choices = []
    # Look for #N format within the text
    matches = re.finditer(r'#(\d+)', text)
    for match in matches:
        section_id = match.group(1)
        if section_id not in choices:
            choices.append(section_id)
    return choices

def display_text_with_choices(text):
    """Display the text with choice markers highlighted."""
    # Replace #N with [N] to highlight choices
    highlighted_text = re.sub(r'#(\d+)', r'[\1]', text)
    return highlighted_text

def play_gamebook(filename):
    """Play the gamebook loaded from the given file."""
    # Get the directory of the currently running script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, filename)
    
    # Load gamebook using the absolute path
    """Play the gamebook loaded from the given file."""
    sections = load_gamebook(file_path)
    
    if not sections:
        print("Failed to load gamebook or the file is empty. Exiting.")
        return
    
    current_section = "1"  # Start with section 1
    
    while True:
        # Display current section
        if current_section not in sections:
            print(f"Error: Section {current_section} not found.")
            break
            
        section_text = sections[current_section]
        print("\n" + "=" * 50)
        print(f"Section {current_section}:")
        print("=" * 50)
        print(display_text_with_choices(section_text))
        
        # Find choices
        choices = find_choices(section_text)
        
        if not choices:
            print("\nTHE END - No more choices available.")
            break
        
        # Display choices
        print("\nAvailable choices:", ", ".join(choices))
        
        # Get user choice
        while True:
            choice = input("\nEnter your choice (section number): ").strip()
            
            if choice in choices:
                current_section = choice
                break
            elif choice.lower() == 'quit':
                print("Thanks for playing!")
                return
            else:
                print("Invalid choice. Try again or type 'quit' to exit.")

if __name__ == "__main__":
    gamebook_file = input("Enter the filename of the gamebook: ").strip()
    play_gamebook(gamebook_file)
