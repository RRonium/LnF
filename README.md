# LnF - Lost and Found System (Interactive UI)

A high-performance terminal application built in **C** using the **ncurses** library. This system manages lost and found items using advanced Data Structures and Algorithms (DSA), featuring a fully interactive menu-driven interface.

## 🚀 Features
- **Interactive UI**: Navigate menus using **Arrow Keys** and **Enter**.
- **Persistent Storage**: Data is serialized and saved to `database.txt`.
- **Multi-Layer Search**: 
  - **Exact Match**: Hash Table lookup.
  - **Substring Match**: Rabin-Karp Algorithm.
  - **Fuzzy Match**: Levenshtein Distance (handles typos).
- **Secure Reclaim**: Identity verification using Unique IDs and Fuzzy-matched Secret Details.

---

## 🧠 Algorithms & Data Structures

### 1. Hash Table with Separate Chaining
- **Storage**: Items are indexed via a **Polynomial Rolling Hash** function.
- **Complexity**: Average case $O(1)$ for insertion and lookup.
- **Collisions**: Handled using Linked Lists (Chaining) to ensure no data loss.

### 2. Rabin-Karp Algorithm
- Used for searching keywords within long item descriptions.
- Employs a **Rolling Hash** to find patterns in $O(n+m)$ time, significantly faster than brute-force string matching.

### 3. Levenshtein Distance (Fuzzy Logic)
- Implemented via **Dynamic Programming**.
- Calculates the "Edit Distance" between strings to allow for user typos during search and security verification (threshold $\le 3$).

---

## 🛠️ Installation & Execution

Since this project uses the `ncurses` library, you must link it during compilation.

### Prerequisites
- **MacOS**: ncurses is pre-installed.
- **Linux**: Install via `sudo apt-get install libncurses5-dev libncursesw5-dev` if not present.

### Compilation
Open your terminal in the project folder and run:
```bash
gcc main.c -o LnF -lncurses