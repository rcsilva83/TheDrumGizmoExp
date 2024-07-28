import tkinter as tk
import jack

client = jack.Client("py_hihat")

outport = client.midi_outports.register("out")
send_queue = []

@client.set_process_callback
def process(frames):
    outport.clear_buffer()
    for event in send_queue:
        outport.write_midi_event(0, event)
    send_queue.clear()

class HiHatApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Hi-hat Controller")
        
        # Set a fixed size for the window
        self.root.geometry("200x500")

        self.state = 64
        self.max_state = 127

        self.instruction_label = tk.Label(root, text="Scroll the mouse wheel to change the hi-hat position. Click to hit. Press 'q' to quit.", font=("Helvetica", 10), wraplength=180, justify="center")
        self.instruction_label.pack(pady=10)

        self.position_label = tk.Label(root, text=f"Hi-hat position: {self.state}", font=("Helvetica", 14))
        self.position_label.pack(pady=5)

        self.canvas = tk.Canvas(root, width=100, height=300, bg='white')
        self.canvas.pack(pady=10)

        self.hit_label = tk.Label(root, text="", font=("Helvetica", 24), fg='red')
        self.hit_label.pack()

        self.update_progress_bar()

        # Bind mouse wheel events and button click event
        self.canvas.bind("<Button-1>", self.on_left_click)
        self.root.bind("<Key>", self.on_key_press)
        self.canvas.bind("<MouseWheel>", self.on_mouse_wheel)  # For Windows and MacOS
        self.canvas.bind("<Button-4>", self.on_mouse_wheel_up)  # For Linux
        self.canvas.bind("<Button-5>", self.on_mouse_wheel_down)  # For Linux

    def update_progress_bar(self):
        self.canvas.delete("all")
        progress_height = int((self.state / self.max_state) * 300)
        self.canvas.create_rectangle(40, 300 - progress_height, 60, 300, fill="blue")
        self.hit_label.config(text="")

    def transmit(self, msg):
        send_queue.append(msg)

    def transmit_hit(self):
        self.transmit_state()
        self.transmit((0x90, 26, 100))
        
    def transmit_state(self):
        self.transmit((0xB0, 4, self.state))
    
    def adjust_openness(self, amount):
        self.state = max(0, min(self.max_state, self.state + amount))

    def on_mouse_wheel(self, event):
        if event.delta > 0:
            self.adjust_openness(5)
        else:
            self.adjust_openness(-5)
        self.transmit_state()
        self.position_label.config(text=f"Hi-hat position: {self.state}")
        self.update_progress_bar()

    def on_mouse_wheel_up(self, event):
        self.adjust_openness(5)
        self.transmit_state()
        self.position_label.config(text=f"Hi-hat position: {self.state}")
        self.update_progress_bar()

    def on_mouse_wheel_down(self, event):
        self.adjust_openness(-5)
        self.transmit_state()
        self.position_label.config(text=f"Hi-hat position: {self.state}")
        self.update_progress_bar()

    def on_left_click(self, event):
        self.transmit_hit()
        self.hit_label.config(text="hit")
        self.root.after(200, self.clear_hit_label)

    def clear_hit_label(self):
        self.hit_label.config(text="")

    def on_key_press(self, event):
        if event.char == 'q':
            self.root.quit()

if __name__ == "__main__":
    with client:
        root = tk.Tk()
        app = HiHatApp(root)
        root.mainloop()
