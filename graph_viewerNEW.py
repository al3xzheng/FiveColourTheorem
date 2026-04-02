import tkinter as tk
import math
import random

# ── PASTE YOUR ADJACENCY LIST HERE ──────────────────────────────────────────
ADJACENCY_LIST = [
    [1, 2],
    [0, 3, 4],
    [0, 4],
    [1, 5],
    [1, 2, 5],
    [3, 4],
]
# ────────────────────────────────────────────────────────────────────────────

BG          = "#0a0c12"
CANVAS_BG   = "#0d1017"
PANEL_BG    = "#111520"
PANEL2_BG   = "#161b28"
BORDER      = "#1e2840"
TEXT_DIM    = "#4a5568"
TEXT_MID    = "#8892a4"
TEXT_BRIGHT = "#c8d0e0"
ACCENT      = "#4070ff"
ACCENT2     = "#00d4aa"
EDGE_COL    = "#1e2e50"
EDGE_HOV    = "#3a5080"
LABEL_FG    = "#e8eaf6"

NODE_COLORS = {
    "default": "#1a2540",
    "red":     "#c0392b",
    "green":   "#27ae60",
    "blue":    "#2474a8",
    "yellow":  "#b8960a",
    "white":   "#d0d8f0",
}
NODE_STROKES = {
    "default": "#2e4080",
    "red":     "#e74c3c",
    "green":   "#2ecc71",
    "blue":    "#3498db",
    "yellow":  "#f1c40f",
    "white":   "#ffffff",
}

NODE_R = 24
FONT   = "Courier"


def circular_layout(n, cx, cy, r):
    pts = []
    for i in range(n):
        a = 2 * math.pi * i / n - math.pi / 2
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


class ColorPopup:
    PALETTE = [
        ("red",    "#e74c3c", "R"),
        ("green",  "#2ecc71", "G"),
        ("blue",   "#3498db", "B"),
        ("yellow", "#f1c40f", "Y"),
        ("white",  "#e8eaf6", "W"),
        ("default","#4070ff", "x"),
    ]

    def __init__(self, canvas, cx, cy, node_id, on_pick, on_delete=None):
        self.top = tk.Toplevel(canvas)
        self.top.overrideredirect(True)
        self.top.attributes("-topmost", True)
        self.top.configure(bg=BORDER)

        wrapper = tk.Frame(self.top, bg=PANEL_BG, padx=12, pady=10)
        wrapper.pack(padx=1, pady=1)

        tk.Label(wrapper, text="Node %d" % node_id,
                 font=(FONT, 9, "bold"),
                 fg=TEXT_MID, bg=PANEL_BG).pack(anchor="w", pady=(0, 6))

        # Color swatches row
        row = tk.Frame(wrapper, bg=PANEL_BG)
        row.pack()

        for key, hex_col, lbl in self.PALETTE:
            c = tk.Canvas(row, width=32, height=32, bg=PANEL_BG,
                          highlightthickness=0, cursor="hand2")
            c.pack(side="left", padx=3)
            c.create_oval(3, 3, 29, 29, fill=hex_col, outline="", width=0)
            c.create_text(16, 16, text=lbl,
                          font=(FONT, 9, "bold"), fill="white")
            c.bind("<Button-1>", lambda e, k=key: self._pick(k, on_pick))
            c.bind("<Enter>",  lambda e, w=c: w.configure(bg="#1e2640"))
            c.bind("<Leave>",  lambda e, w=c: w.configure(bg=PANEL_BG))

        # Divider
        tk.Frame(wrapper, bg=BORDER, height=1).pack(fill="x", pady=(8, 6))

        # Delete node button
        del_btn = tk.Button(wrapper, text="✕  Delete Node",
                            font=(FONT, 8, "bold"),
                            bg="#2a0a0a", fg="#e74c3c",
                            relief="flat", bd=0, cursor="hand2",
                            activebackground="#3d1010",
                            activeforeground="#ff6b6b",
                            pady=5,
                            command=lambda: self._delete(on_delete))
        del_btn.pack(fill="x")

        self.top.update_idletasks()
        pw = self.top.winfo_reqwidth()
        ph = self.top.winfo_reqheight()
        rx = canvas.winfo_rootx() + int(cx) - pw // 2
        ry = canvas.winfo_rooty() + int(cy) - NODE_R - ph - 10
        self.top.geometry("+%d+%d" % (rx, ry))
        self.top.bind("<FocusOut>", lambda e: self.close())
        self.top.focus_force()

    def _pick(self, key, on_pick):
        self.close()
        on_pick(key)

    def _delete(self, on_delete):
        self.close()
        if on_delete:
            on_delete()

    def close(self):
        try:
            self.top.destroy()
        except Exception:
            pass


class GraphApp:
    def __init__(self, root, adj):
        self.root = root
        self.adj = [list(nb) for nb in adj]
        self.n = len(self.adj)
        self.colors = ["default"] * self.n
        self.pos = []
        self.dragging = None
        self.drag_offset = (0, 0)
        self.hover_node = None
        self.hover_edge = None
        self.edge_mode   = False
        self.edge_src    = None
        self.delete_mode = False
        self.popup       = None

        root.title("Graph Visualizer")
        root.configure(bg=BG)
        root.minsize(920, 640)
        self._build_ui()
        self._init_pos()
        self._draw()

    def _make_scrollable_panel(self, side, width):
        """Create a fixed-width panel with a scrollable inner frame."""
        outer = tk.Frame(self.root, bg=PANEL_BG, width=width,
                         highlightbackground=BORDER, highlightthickness=1)
        outer.pack(side=side, fill="y")
        outer.pack_propagate(False)

        scrl = tk.Scrollbar(outer, orient="vertical", width=6,
                            bg=PANEL_BG, troughcolor=PANEL_BG,
                            highlightthickness=0)
        scrl.pack(side="right", fill="y")

        cvs = tk.Canvas(outer, bg=PANEL_BG, highlightthickness=0,
                        yscrollcommand=scrl.set)
        cvs.pack(side="left", fill="both", expand=True)
        scrl.config(command=cvs.yview)

        inner = tk.Frame(cvs, bg=PANEL_BG)
        win_id = cvs.create_window((0, 0), window=inner, anchor="nw")

        def _on_inner_configure(e):
            cvs.configure(scrollregion=cvs.bbox("all"))

        def _on_canvas_configure(e):
            cvs.itemconfig(win_id, width=e.width)

        inner.bind("<Configure>", _on_inner_configure)
        cvs.bind("<Configure>", _on_canvas_configure)

        def _on_mousewheel(e):
            cvs.yview_scroll(int(-1 * (e.delta / 120)), "units")

        cvs.bind("<MouseWheel>", _on_mousewheel)
        inner.bind("<MouseWheel>", _on_mousewheel)

        return inner

    def _build_ui(self):
        # ── Left scrollable panel ─────────────────────────────────────────────
        sb = self._make_scrollable_panel("left", 250)

        def sec(txt, parent=None):
            p = parent or sb
            tk.Label(p, text=txt, font=(FONT, 8, "bold"),
                     fg=TEXT_DIM, bg=PANEL_BG, anchor="w").pack(
                fill="x", padx=14, pady=(14, 3))

        def div(parent=None):
            p = parent or sb
            tk.Frame(p, bg=BORDER, height=1).pack(fill="x", padx=8, pady=4)

        def btn(txt, cmd, fg=TEXT_BRIGHT, bg=PANEL2_BG, parent=None):
            p = parent or sb
            b = tk.Button(p, text=txt, font=(FONT, 9, "bold"),
                          bg=bg, fg=fg, relief="flat", bd=0,
                          activebackground=BORDER,
                          activeforeground=TEXT_BRIGHT,
                          cursor="hand2", pady=7, command=cmd)
            b.pack(fill="x", padx=14, pady=(5, 0))
            return b

        def entry(var, parent=None):
            p = parent or sb
            e = tk.Entry(p, textvariable=var, font=(FONT, 10),
                         bg=PANEL2_BG, fg=TEXT_BRIGHT,
                         insertbackground=ACCENT,
                         relief="flat", bd=0,
                         highlightbackground=BORDER,
                         highlightthickness=1, width=8)
            e.pack(fill="x", padx=14, pady=(2, 0), ipady=5)
            return e

        # Stats
        sec("GRAPH")
        self.stat_lbl = tk.Label(sb, text="", font=(FONT, 9),
                                 fg=TEXT_MID, bg=PANEL_BG,
                                 justify="left", anchor="w")
        self.stat_lbl.pack(fill="x", padx=14, pady=(0, 6))
        div()

        # Add node
        sec("ADD NODE")
        btn("+ Add Node", self._add_node, fg=ACCENT2, bg=PANEL2_BG)
        div()

        # Add edge typed
        sec("ADD EDGE  (type)")
        tk.Label(sb, text="From:", font=(FONT, 9), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14)
        self.ef_var = tk.StringVar()
        entry(self.ef_var)
        tk.Label(sb, text="To:", font=(FONT, 9), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14, pady=(6, 0))
        self.et_var = tk.StringVar()
        entry(self.et_var)
        btn("+ Add Edge", self._add_edge_typed, fg=ACCENT, bg=PANEL2_BG)
        div()

        # Draw mode
        sec("ADD EDGE  (draw)")
        self.dm_btn = btn("Pen  Draw Mode: OFF", self._toggle_draw,
                          fg=TEXT_MID, bg=PANEL2_BG)
        div()

        # Delete mode
        sec("DELETE")
        self.del_btn = btn("X  Delete Mode: OFF", self._toggle_delete,
                           fg=TEXT_MID, bg=PANEL2_BG)
        tk.Label(sb, text="Click node or edge to delete",
                 font=(FONT, 8), fg=TEXT_DIM, bg=PANEL_BG,
                 anchor="w").pack(fill="x", padx=14, pady=(3, 0))

        # Typed edge delete
        sec("DELETE EDGE  (type)")
        tk.Label(sb, text="From:", font=(FONT, 9), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14)
        self.def_var = tk.StringVar()
        entry(self.def_var)
        tk.Label(sb, text="To:", font=(FONT, 9), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14, pady=(6, 0))
        self.det_var = tk.StringVar()
        entry(self.det_var)
        btn("- Delete Edge", self._delete_edge_typed, fg="#e74c3c", bg=PANEL2_BG)
        div()

        # Tips
        sec("TIPS")
        hints = [
            "Left-drag   -> move node",
            "Right-click -> color/delete node",
            "Draw mode   -> click 2 nodes",
            "Delete mode -> click node/edge",
            "Scroll left panel for export",
        ]
        for h in hints:
            tk.Label(sb, text=h, font=(FONT, 8), fg=TEXT_DIM,
                     bg=PANEL_BG, anchor="w").pack(fill="x", padx=14, pady=1)
        div()
        btn("Reset Layout", self._reset, fg=TEXT_MID, bg=PANEL2_BG)
        div()

        # ── EXPORT GRAPH (on left panel, always visible via scroll) ───────────
        sec("EXPORT GRAPH")

        # Tab row
        tab_row = tk.Frame(sb, bg=PANEL_BG)
        tab_row.pack(fill="x", padx=14, pady=(0, 6))
        self._export_tab = tk.StringVar(value="cpp")

        def make_tab(label, value):
            def select():
                self._export_tab.set(value)
                for w, v in tab_btns:
                    active = self._export_tab.get() == v
                    w.config(fg=ACCENT if active else TEXT_DIM,
                             bg="#0d1520" if active else PANEL2_BG)
                self._refresh_export()
            b = tk.Button(tab_row, text=label, font=(FONT, 8, "bold"),
                          fg=ACCENT if value == "cpp" else TEXT_DIM,
                          bg="#0d1520" if value == "cpp" else PANEL2_BG,
                          relief="flat", bd=0, cursor="hand2",
                          activebackground=BORDER, padx=6, pady=4,
                          command=select)
            b.pack(side="left", padx=(0, 3))
            return b

        tab_btns = []
        b1 = make_tab("C++",    "cpp")
        b2 = make_tab("Plain",  "plain")
        b3 = make_tab("Colors", "colors")
        tab_btns = [(b1, "cpp"), (b2, "plain"), (b3, "colors")]

        exp_frame = tk.Frame(sb, bg=BORDER, padx=1, pady=1)
        exp_frame.pack(fill="x", padx=14)

        exp_inner = tk.Frame(exp_frame, bg="#090b10")
        exp_inner.pack(fill="x")

        exp_yscroll = tk.Scrollbar(exp_inner, orient="vertical", width=6,
                                   bg=PANEL2_BG, troughcolor="#090b10")
        exp_yscroll.pack(side="right", fill="y")

        exp_xscroll = tk.Scrollbar(exp_frame, orient="horizontal", width=6,
                                   bg=PANEL2_BG, troughcolor="#090b10")
        exp_xscroll.pack(fill="x")

        self.export_text = tk.Text(exp_inner, font=(FONT, 8),
                                   bg="#090b10", fg=ACCENT2,
                                   relief="flat", bd=0,
                                   height=10, wrap="none",
                                   highlightthickness=0,
                                   state="disabled",
                                   yscrollcommand=exp_yscroll.set,
                                   xscrollcommand=exp_xscroll.set)
        self.export_text.pack(fill="x", padx=4, pady=4)
        exp_yscroll.config(command=self.export_text.yview)
        exp_xscroll.config(command=self.export_text.xview)

        self.export_info = tk.Label(sb, text="",
                                    font=(FONT, 8), fg=TEXT_DIM,
                                    bg=PANEL_BG, anchor="w",
                                    wraplength=220, justify="left")
        self.export_info.pack(fill="x", padx=14, pady=(3, 0))

        btn("Refresh Export", self._refresh_export, fg=TEXT_MID, bg=PANEL2_BG)
        btn("Copy to Clipboard", self._copy_export, fg=ACCENT, bg=PANEL2_BG)
        tk.Frame(sb, bg=BG, height=20).pack()   # bottom padding

        # ── Centre canvas area ────────────────────────────────────────────────
        main = tk.Frame(self.root, bg=BG)
        main.pack(side="left", fill="both", expand=True)

        topbar = tk.Frame(main, bg=PANEL_BG, height=46,
                          highlightbackground=BORDER, highlightthickness=1)
        topbar.pack(fill="x")
        topbar.pack_propagate(False)

        tk.Label(topbar, text="GRAPH VISUALIZER",
                 font=(FONT, 13, "bold"),
                 fg=ACCENT, bg=PANEL_BG).pack(side="left", padx=18, pady=12)

        self.mode_lbl = tk.Label(topbar, text="",
                                 font=(FONT, 9, "bold"),
                                 fg=ACCENT2, bg=PANEL_BG)
        self.mode_lbl.pack(side="right", padx=18)

        self.canvas = tk.Canvas(main, bg=CANVAS_BG, highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)

        statusbar = tk.Frame(main, bg=PANEL_BG, height=28,
                             highlightbackground=BORDER, highlightthickness=1)
        statusbar.pack(fill="x", side="bottom")
        statusbar.pack_propagate(False)
        self.status_var = tk.StringVar(value="Ready.")
        tk.Label(statusbar, textvariable=self.status_var,
                 font=(FONT, 8), fg=TEXT_DIM, bg=PANEL_BG,
                 anchor="w").pack(side="left", padx=12, pady=5)

        # ── Right scrollable panel ────────────────────────────────────────────
        rb = self._make_scrollable_panel("right", 250)

        def rsec(txt):
            tk.Label(rb, text=txt, font=(FONT, 8, "bold"),
                     fg=TEXT_DIM, bg=PANEL_BG, anchor="w").pack(
                fill="x", padx=14, pady=(14, 4))

        def rdiv():
            tk.Frame(rb, bg=BORDER, height=1).pack(fill="x", padx=8, pady=6)

        def rbtn(txt, cmd, fg=TEXT_BRIGHT, bg=PANEL2_BG):
            b = tk.Button(rb, text=txt, font=(FONT, 9, "bold"),
                          bg=bg, fg=fg, relief="flat", bd=0,
                          activebackground=BORDER,
                          activeforeground=TEXT_BRIGHT,
                          cursor="hand2", pady=7, command=cmd)
            b.pack(fill="x", padx=14, pady=(4, 0))
            return b

        # Color sequence
        rsec("COLOR SEQUENCE")

        legend_frame = tk.Frame(rb, bg=PANEL_BG)
        legend_frame.pack(fill="x", padx=14, pady=(0, 6))
        chips = [("1", NODE_STROKES["red"],    "red"),
                 ("2", NODE_STROKES["green"],  "green"),
                 ("3", NODE_STROKES["blue"],   "blue"),
                 ("4", NODE_STROKES["yellow"], "yellow"),
                 ("5", NODE_STROKES["white"],  "white")]
        for code, col, name in chips:
            row = tk.Frame(legend_frame, bg=PANEL_BG)
            row.pack(fill="x", pady=1)
            chip = tk.Canvas(row, width=14, height=14, bg=PANEL_BG,
                             highlightthickness=0)
            chip.pack(side="left", padx=(0, 6))
            chip.create_oval(1, 1, 13, 13, fill=NODE_COLORS[name],
                             outline=col, width=1)
            tk.Label(row, text="%s = %s" % (code, name),
                     font=(FONT, 8), fg=TEXT_MID,
                     bg=PANEL_BG, anchor="w").pack(side="left")

        rdiv()

        tk.Label(rb, text="Paste sequence (space-separated):",
                 font=(FONT, 8), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w",
                 wraplength=220, justify="left").pack(
            fill="x", padx=14, pady=(2, 4))

        tk.Label(rb, text='e.g.  "1 1 2 3 4 5"',
                 font=(FONT, 8, "italic"), fg=TEXT_DIM,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14, pady=(0, 6))

        txt_frame = tk.Frame(rb, bg=BORDER, padx=1, pady=1)
        txt_frame.pack(fill="x", padx=14)
        self.seq_text = tk.Text(txt_frame, font=(FONT, 10),
                                bg=PANEL2_BG, fg=TEXT_BRIGHT,
                                insertbackground=ACCENT,
                                relief="flat", bd=0,
                                height=4, wrap="word",
                                highlightthickness=0)
        self.seq_text.pack(fill="x", padx=4, pady=4)

        rbtn("Apply Colors", self._apply_color_sequence,
             fg=ACCENT, bg=PANEL2_BG)
        rbtn("Clear All Colors", self._clear_colors,
             fg=TEXT_MID, bg=PANEL2_BG)

        rdiv()

        rsec("PREVIEW")
        self.preview_canvas = tk.Canvas(rb, bg=PANEL2_BG,
                                        height=60, highlightthickness=0)
        self.preview_canvas.pack(fill="x", padx=14, pady=(0, 6))

        self.seq_feedback = tk.Label(rb, text="",
                                     font=(FONT, 8), fg=TEXT_MID,
                                     bg=PANEL_BG, anchor="w",
                                     wraplength=220, justify="left")
        self.seq_feedback.pack(fill="x", padx=14)

        self.seq_text.bind("<KeyRelease>", lambda e: self._preview_sequence())

        rdiv()

        # Import graph
        rsec("IMPORT GRAPH")

        tk.Label(rb, text="Paste adjacency matrix\n(C++ or plain 0/1 format):",
                 font=(FONT, 8), fg=TEXT_MID,
                 bg=PANEL_BG, anchor="w").pack(fill="x", padx=14, pady=(0, 4))

        imp_frame = tk.Frame(rb, bg=BORDER, padx=1, pady=1)
        imp_frame.pack(fill="x", padx=14)
        self.import_text = tk.Text(imp_frame, font=(FONT, 8),
                                   bg=PANEL2_BG, fg=TEXT_BRIGHT,
                                   insertbackground=ACCENT,
                                   relief="flat", bd=0,
                                   height=7, wrap="none",
                                   highlightthickness=0)
        imp_sb_w = tk.Scrollbar(imp_frame, orient="vertical", width=6,
                                command=self.import_text.yview,
                                bg=PANEL2_BG, troughcolor=PANEL2_BG)
        self.import_text.configure(yscrollcommand=imp_sb_w.set)
        imp_sb_w.pack(side="right", fill="y")
        self.import_text.pack(fill="x", padx=4, pady=4)

        self.import_feedback = tk.Label(rb, text="",
                                        font=(FONT, 8), fg=TEXT_MID,
                                        bg=PANEL_BG, anchor="w",
                                        wraplength=220, justify="left")
        self.import_feedback.pack(fill="x", padx=14, pady=(3, 0))

        rbtn("Load Graph", self._import_graph, fg=ACCENT2, bg=PANEL2_BG)
        tk.Frame(rb, bg=BG, height=20).pack()   # bottom padding

        self.canvas.bind("<ButtonPress-1>",   self._press)
        self.canvas.bind("<B1-Motion>",       self._drag)
        self.canvas.bind("<ButtonRelease-1>", self._release)
        self.canvas.bind("<Button-3>",        self._right_click)
        self.canvas.bind("<Motion>",          self._hover)
        self.canvas.bind("<Configure>",       lambda e: self._draw())

    def _init_pos(self):
        w = max(self.canvas.winfo_width(), 600)
        h = max(self.canvas.winfo_height(), 500)
        r = min(w, h) * 0.36
        self.pos = circular_layout(self.n, w / 2, h / 2, r)

    def _reset(self):
        self._init_pos()
        self._draw()
        self.status("Layout reset.")

    def _add_node(self):
        nid = self.n
        self.adj.append([])
        self.colors.append("default")
        self.n += 1
        w = max(self.canvas.winfo_width(), 600)
        h = max(self.canvas.winfo_height(), 500)
        a = random.uniform(0, 2 * math.pi)
        r = random.uniform(60, min(w, h) * 0.3)
        self.pos.append((w / 2 + r * math.cos(a), h / 2 + r * math.sin(a)))
        self._draw()
        self.status("Node %d added." % nid)

    def _add_edge_typed(self):
        try:
            u = int(self.ef_var.get())
            v = int(self.et_var.get())
        except ValueError:
            self.status("Enter valid integers.")
            return
        self._connect(u, v)
        self.ef_var.set("")
        self.et_var.set("")

    def _connect(self, u, v):
        if not (0 <= u < self.n and 0 <= v < self.n):
            self.status("Nodes must be 0-%d." % (self.n - 1))
            return
        if u == v:
            self.status("No self-loops.")
            return
        if v in self.adj[u]:
            self.status("Edge %d-%d already exists." % (u, v))
            return
        self.adj[u].append(v)
        self.adj[v].append(u)
        self._draw()
        self.status("Edge %d - %d added." % (u, v))

    def _toggle_draw(self):
        self.edge_mode = not self.edge_mode
        self.edge_src = None
        if self.edge_mode:
            # Turn off delete mode if on
            if self.delete_mode:
                self._toggle_delete()
            self.dm_btn.config(text="Pen  Draw Mode: ON",
                               fg=ACCENT2, bg="#0d201a")
            self.mode_lbl.config(text="EDGE DRAW MODE")
            self.canvas.configure(cursor="crosshair")
            self.status("Click source node, then target node.")
        else:
            self.dm_btn.config(text="Pen  Draw Mode: OFF",
                               fg=TEXT_MID, bg=PANEL2_BG)
            self.mode_lbl.config(text="")
            self.canvas.configure(cursor="")
            self.status("Draw mode off.")
        self._draw()

    def _toggle_delete(self):
        self.delete_mode = not self.delete_mode
        if self.delete_mode:
            # Turn off draw mode if on
            if self.edge_mode:
                self.edge_mode = False
                self.edge_src = None
                self.dm_btn.config(text="Pen  Draw Mode: OFF",
                                   fg=TEXT_MID, bg=PANEL2_BG)
            self.del_btn.config(text="X  Delete Mode: ON",
                                fg="#e74c3c", bg="#200808")
            self.mode_lbl.config(text="DELETE MODE")
            self.canvas.configure(cursor="X_cursor")
            self.status("Click a node to delete it (+ its edges), or click an edge to remove it.")
        else:
            self.del_btn.config(text="X  Delete Mode: OFF",
                                fg=TEXT_MID, bg=PANEL2_BG)
            self.mode_lbl.config(text="")
            self.canvas.configure(cursor="")
            self.status("Delete mode off.")
        self._draw()

    def _delete_node(self, i):
        """Remove node i, all its edges, and remap remaining indices."""
        removed_edges = len(self.adj[i])
        # Remove from pos, colors, adj
        del self.pos[i]
        del self.colors[i]
        del self.adj[i]
        self.n -= 1
        # Remap all remaining adjacency lists
        new_adj = []
        for nbrs in self.adj:
            new_nbrs = []
            for j in nbrs:
                if j == i:
                    continue        # edge to deleted node — drop it
                elif j > i:
                    new_nbrs.append(j - 1)
                else:
                    new_nbrs.append(j)
            new_adj.append(new_nbrs)
        self.adj = new_adj
        # Fix hover / edge_src references
        self.hover_node = None
        self.hover_edge = None
        if self.edge_src == i:
            self.edge_src = None
        elif self.edge_src is not None and self.edge_src > i:
            self.edge_src -= 1
        self._draw()
        self.status("Deleted node %d and %d edge(s)." % (i, removed_edges))

    def _delete_edge(self, u, v):
        """Remove the edge between u and v."""
        if v in self.adj[u]:
            self.adj[u].remove(v)
        if u in self.adj[v]:
            self.adj[v].remove(u)
        self.hover_edge = None
        self._draw()
        self.status("Deleted edge %d - %d." % (u, v))

    def _delete_edge_typed(self):
        try:
            u = int(self.def_var.get())
            v = int(self.det_var.get())
        except ValueError:
            self.status("Enter valid integers.")
            return
        if not (0 <= u < self.n and 0 <= v < self.n):
            self.status("Nodes must be 0-%d." % (self.n - 1))
            return
        if v not in self.adj[u]:
            self.status("Edge %d-%d does not exist." % (u, v))
            return
        self._delete_edge(u, v)
        self.def_var.set("")
        self.det_var.set("")

    def _draw(self):
        c = self.canvas
        c.delete("all")

        w, h = c.winfo_width(), c.winfo_height()
        for gx in range(0, w + 40, 40):
            for gy in range(0, h + 40, 40):
                c.create_oval(gx-1, gy-1, gx+1, gy+1,
                              fill="#141820", outline="")

        for u in range(self.n):
            for v in self.adj[u]:
                if v > u:
                    x1, y1 = self.pos[u]
                    x2, y2 = self.pos[v]
                    is_hov = self.hover_edge == (u, v)
                    is_del = self.delete_mode and is_hov
                    if is_del:
                        col, w = "#e74c3c", 3.0
                    elif is_hov:
                        col, w = EDGE_HOV, 2.5
                    else:
                        col, w = EDGE_COL, 1.5
                    c.create_line(x1, y1, x2, y2,
                                  fill=col, width=w,
                                  capstyle="round")

        if self.edge_mode and self.edge_src is not None:
            sx, sy = self.pos[self.edge_src]
            c.create_oval(sx-NODE_R-8, sy-NODE_R-8,
                          sx+NODE_R+8, sy+NODE_R+8,
                          outline=ACCENT2, width=2, dash=(5, 3), fill="")

        for i, (x, y) in enumerate(self.pos):
            ck     = self.colors[i]
            fill   = NODE_COLORS[ck]
            stroke = NODE_STROKES[ck]
            is_hov = (i == self.hover_node)
            is_src = (i == self.edge_src)

            is_del_hov = self.delete_mode and i == self.hover_node
            if is_hov or is_src or is_del_hov:
                if is_del_hov:
                    gc = "#e74c3c"
                elif is_src:
                    gc = ACCENT2
                else:
                    gc = "#6090ff"
                c.create_oval(x-NODE_R-8, y-NODE_R-8,
                              x+NODE_R+8, y+NODE_R+8,
                              outline=gc, width=2.0, fill="")

            c.create_oval(x-NODE_R, y-NODE_R,
                          x+NODE_R, y+NODE_R,
                          fill=fill, outline=stroke, width=2)
            label_color = "#1a2030" if ck == "white" else LABEL_FG
            c.create_text(x, y, text=str(i),
                          font=(FONT, 11, "bold"), fill=label_color)

        edges   = sum(len(nb) for nb in self.adj) // 2
        colored = sum(1 for col in self.colors if col != "default")
        self.stat_lbl.config(
            text="  Nodes  : %d\n  Edges  : %d\n  Colored: %d" % (self.n, edges, colored))

    def _node_at(self, x, y):
        for i, (nx, ny) in enumerate(self.pos):
            if math.hypot(x-nx, y-ny) <= NODE_R + 4:
                return i
        return None

    def _edge_at(self, x, y, thresh=8):
        for u in range(self.n):
            for v in self.adj[u]:
                if v > u:
                    x1, y1 = self.pos[u]
                    x2, y2 = self.pos[v]
                    dx, dy = x2-x1, y2-y1
                    if dx == dy == 0:
                        continue
                    t = max(0, min(1, ((x-x1)*dx+(y-y1)*dy)/(dx*dx+dy*dy)))
                    if math.hypot(x-(x1+t*dx), y-(y1+t*dy)) <= thresh:
                        return (u, v)
        return None

    def _press(self, event):
        node = self._node_at(event.x, event.y)

        if self.delete_mode:
            if node is not None:
                self._delete_node(node)
            else:
                edge = self._edge_at(event.x, event.y)
                if edge:
                    self._delete_edge(*edge)
            return

        if self.edge_mode:
            if node is not None:
                if self.edge_src is None:
                    self.edge_src = node
                    self.status("Node %d selected -> click target." % node)
                    self._draw()
                else:
                    self._connect(self.edge_src, node)
                    self.edge_src = None
            return
        if node is not None:
            self.dragging = node
            nx, ny = self.pos[node]
            self.drag_offset = (nx - event.x, ny - event.y)

    def _drag(self, event):
        if self.dragging is not None and not self.edge_mode:
            ox, oy = self.drag_offset
            self.pos[self.dragging] = (event.x + ox, event.y + oy)
            self._draw()

    def _release(self, event):
        self.dragging = None

    def _right_click(self, event):
        node = self._node_at(event.x, event.y)
        if node is None:
            # right-click on edge → delete edge directly
            edge = self._edge_at(event.x, event.y)
            if edge:
                u, v = edge
                self._delete_edge(u, v)
            return
        if self.popup:
            self.popup.close()
        x, y = self.pos[node]
        self.popup = ColorPopup(
            self.canvas, x, y, node,
            on_pick=lambda col, n=node: self._set_color(n, col),
            on_delete=lambda n=node: self._delete_node(n))

    def _set_color(self, node, color):
        self.colors[node] = color
        self._draw()
        self.status("Node %d -> %s." % (node, color))

    def _hover(self, event):
        node = self._node_at(event.x, event.y)
        edge = None if node is not None else self._edge_at(event.x, event.y)
        changed = (node != self.hover_node or edge != self.hover_edge)
        self.hover_node = node
        self.hover_edge = edge

        if self.delete_mode:
            if node is not None:
                self.status("Click to DELETE node %d  (and its %d edges)" % (node, len(self.adj[node])))
                self.canvas.configure(cursor="X_cursor")
            elif edge:
                u, v = edge
                self.status("Click to DELETE edge %d - %d" % (u, v))
                self.canvas.configure(cursor="X_cursor")
            else:
                self.status("Delete Mode: click a node or edge to remove it.")
                self.canvas.configure(cursor="")
        elif node is not None:
            self.status("Node %d  neighbours: %s  |  right-click: color/delete  |  drag: move"
                        % (node, self.adj[node]))
            self.canvas.configure(cursor="hand2")
        elif edge:
            u, v = edge
            self.status("Edge %d - %d  |  right-click to delete" % (u, v))
            self.canvas.configure(cursor="")
        else:
            self.status("Left-drag: move  |  Right-click: color/delete  |  Draw/Delete Mode: toolbar")
            if not self.edge_mode:
                self.canvas.configure(cursor="")
        if changed:
            self._draw()

    # ── Color sequence ────────────────────────────────────────────────────────

    CODE_MAP = {"0": "default", "1": "red", "2": "green", "3": "blue", "4": "yellow", "5": "white"}

    def _parse_sequence(self):
        """Return list of color strings from the text box, or None on error."""
        raw = self.seq_text.get("1.0", "end").strip()
        tokens = raw.split()
        result = []
        bad = []
        for tok in tokens:
            if tok in self.CODE_MAP:
                result.append(self.CODE_MAP[tok])
            else:
                bad.append(tok)
        return result, bad

    def _preview_sequence(self):
        colors, bad = self._parse_sequence()
        pc = self.preview_canvas
        pc.delete("all")
        swatch_size = 20
        gap = 4
        x = 6
        for col in colors[:20]:            # show up to 20 chips
            fill   = NODE_COLORS[col]
            stroke = NODE_STROKES[col]
            pc.create_rectangle(x, 6, x + swatch_size, 6 + swatch_size,
                                 fill=fill, outline=stroke, width=1)
            x += swatch_size + gap
            if x > 210:
                break

        if bad:
            self.seq_feedback.config(
                text="Unknown tokens ignored: %s" % " ".join(bad[:8]),
                fg="#e74c3c")
        elif colors:
            extra = ""
            if len(colors) > self.n:
                extra = "  (only first %d used)" % self.n
            elif len(colors) < self.n:
                extra = "  (%d nodes unchanged)" % (self.n - len(colors))
            self.seq_feedback.config(
                text="%d color(s) parsed%s" % (len(colors), extra),
                fg=ACCENT2)
        else:
            self.seq_feedback.config(text="", fg=TEXT_DIM)

    def _apply_color_sequence(self):
        colors, bad = self._parse_sequence()
        if not colors:
            self.status("Nothing to apply — enter codes 1-5 separated by spaces.")
            return
        applied = 0
        for i, col in enumerate(colors):
            if i >= self.n:
                break
            self.colors[i] = col
            applied += 1
        self._preview_sequence()
        self._draw()
        msg = "Applied %d color(s)." % applied
        if bad:
            msg += "  Ignored: %s" % " ".join(bad[:6])
        self.status(msg)

    def _clear_colors(self):
        self.colors = ["default"] * self.n
        self.seq_text.delete("1.0", "end")
        self.preview_canvas.delete("all")
        self.seq_feedback.config(text="")
        self._draw()
        self.status("All colors cleared.")

    def _export_colors(self):
        mapping = {v: k for k, v in self.CODE_MAP.items()}
        codes = [mapping.get(c, "0") for c in self.colors]
        print("Color sequence: " + " ".join(codes))
        print("Colors dict:    " + str({i: self.colors[i] for i in range(self.n)}))
        self.status("Printed color sequence to console.")

    # ── Adjacency matrix import / export ─────────────────────────────────────

    def _adj_list_to_matrix_cpp(self):
        """Return C++-style n×n adjacency matrix (no color column)."""
        n = self.n
        lines = []
        for i in range(n):
            row = [0] * n
            for j in self.adj[i]:
                row[j] = 1
            inner = ", ".join(str(v) for v in row)
            lines.append("        {%s}" % inner)
        return ",\n".join(lines)

    def _adj_list_to_matrix_plain(self):
        """Return plain n×n adjacency matrix as space-separated 0s and 1s."""
        n = self.n
        lines = []
        for i in range(n):
            row = [0] * n
            for j in self.adj[i]:
                row[j] = 1
            lines.append(" ".join(str(v) for v in row))
        return "\n".join(lines)

    def _color_seq_cpp(self):
        """Return C++-style color sequence: {1, 0, 2, 3, ...}"""
        CODE_MAP_REV = {"default": "0", "red": "1", "green": "2",
                        "blue": "3", "yellow": "4", "white": "5"}
        codes = [CODE_MAP_REV.get(c, "0") for c in self.colors]
        return codes  # return list, not string

    def _build_export_text(self):
        tab = getattr(self, "_export_tab", None)
        mode = tab.get() if tab else "cpp"

        if mode == "cpp":
            matrix_str = self._adj_list_to_matrix_cpp()
            body  = "// Adjacency matrix: int adj[%d][%d]\n" % (self.n, self.n)
            body += "// {\n"
            body += matrix_str
            body += "\n// };"
            info = "C++ format  [%d][%d]" % (self.n, self.n)
        elif mode == "plain":
            matrix_str = self._adj_list_to_matrix_plain()
            body  = matrix_str
            info = "Plain format  %d×%d" % (self.n, self.n)
        else:  # colors
            color_codes = self._color_seq_cpp()
            colored = sum(1 for c in self.colors if c != "default")
            body  = "// Color sequence (%d nodes, %d colored)\n" % (self.n, colored)
            body += "// 0=none 1=red 2=green 3=blue 4=yellow 5=white\n"
            body += "// int colors[%d] =\n" % self.n
            body += "        {%s};" % ", ".join(color_codes)
            info = "Colors — %d/%d nodes colored" % (colored, self.n)

        return body, info

    def _refresh_export(self):
        """Rebuild the export text box from current graph state."""
        body, info = self._build_export_text()
        self.export_text.configure(state="normal")
        self.export_text.delete("1.0", "end")
        self.export_text.insert("1.0", body)
        self.export_text.configure(state="disabled")
        if hasattr(self, "export_info"):
            self.export_info.config(text=info)
        self.status("Export refreshed.")

    def _copy_export(self):
        """Copy export text to system clipboard."""
        self._refresh_export()
        raw = self.export_text.get("1.0", "end").strip()
        tab = getattr(self, "_export_tab", None)
        mode = tab.get() if tab else "cpp"
        if mode in ("cpp", "colors"):
            # Strip comment lines for clean C++ paste
            lines = [l for l in raw.splitlines() if not l.strip().startswith("//")]
            clean = "\n".join(lines).strip()
        else:
            clean = raw
        self.root.clipboard_clear()
        self.root.clipboard_append(clean)
        self.root.update()
        self.status("Copied to clipboard.")

    def _parse_cpp_matrix(self, raw):
        """
        Parse a C++-style adjacency matrix like:
            {0, 1, 1, 0},
            {1, 0, 0, 1},
        Returns list-of-lists of ints, or raises ValueError with a message.
        """
        import re
        # Extract each {...} block
        rows_raw = re.findall(r"\{([^}]*)\}", raw)
        if not rows_raw:
            raise ValueError("No rows found. Expected lines like {0, 1, 0, ...}")
        matrix = []
        for i, row_str in enumerate(rows_raw):
            tokens = [t.strip() for t in row_str.split(",") if t.strip() != ""]
            try:
                vals = [int(t) for t in tokens]
            except ValueError:
                raise ValueError("Row %d contains non-integer value." % i)
            matrix.append(vals)

        n = len(matrix)
        for i, row in enumerate(matrix):
            if len(row) != n:
                raise ValueError(
                    "Row %d has %d values but matrix is %dx%d." % (i, len(row), n, n))
        return matrix

    def _parse_plain_matrix(self, raw):
        """
        Parse a plain space-separated adjacency matrix like:
            0 1 1
            1 0 1
            1 1 0
        Returns list-of-lists of ints, or raises ValueError with a message.
        """
        lines = [l.strip() for l in raw.splitlines() if l.strip()]
        matrix = []
        for i, line in enumerate(lines):
            tokens = line.split()
            try:
                vals = [int(t) for t in tokens]
            except ValueError:
                raise ValueError("Row %d contains non-integer value." % i)
            matrix.append(vals)

        if not matrix:
            raise ValueError("No rows found.")
        n = len(matrix)
        for i, row in enumerate(matrix):
            if len(row) != n:
                raise ValueError(
                    "Row %d has %d values but matrix is %dx%d." % (i, len(row), n, n))
        return matrix

    def _parse_matrix_auto(self, raw):
        """Auto-detect C++ or plain format and parse accordingly."""
        if "{" in raw:
            return self._parse_cpp_matrix(raw)
        else:
            return self._parse_plain_matrix(raw)

    def _matrix_to_adj(self, matrix):
        """Convert n×n 0/1 matrix to adjacency list (undirected, ignores diagonal)."""
        n = len(matrix)
        adj = [[] for _ in range(n)]
        for i in range(n):
            for j in range(n):
                if i != j and matrix[i][j] != 0:
                    adj[i].append(j)
        return adj

    def _import_graph(self):
        raw = self.import_text.get("1.0", "end").strip()
        if not raw:
            self.import_feedback.config(
                text="Paste a matrix first.", fg="#e74c3c")
            return
        try:
            matrix = self._parse_matrix_auto(raw)
        except ValueError as e:
            self.import_feedback.config(text="Parse error: %s" % e, fg="#e74c3c")
            return

        new_adj  = self._matrix_to_adj(matrix)
        new_n    = len(new_adj)

        # Rebuild graph state
        self.adj    = new_adj
        self.n      = new_n
        self.colors = ["default"] * new_n
        self.hover_node = None
        self.hover_edge = None
        self.edge_src   = None
        self.dragging   = None

        # Re-layout
        self._init_pos()
        self._refresh_export()
        self._draw()

        self.import_feedback.config(
            text="Loaded %d nodes, %d edges." % (
                new_n, sum(len(r) for r in new_adj) // 2),
            fg=ACCENT2)
        self.status("Graph loaded from matrix (%d nodes)." % new_n)

    # ─────────────────────────────────────────────────────────────────────────

    def status(self, msg):
        self.status_var.set(msg)


def main():
    root = tk.Tk()
    root.geometry("1340x700")
    GraphApp(root, ADJACENCY_LIST)
    root.mainloop()


if __name__ == "__main__":
    main()
