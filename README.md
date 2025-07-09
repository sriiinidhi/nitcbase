# NITCbase: A Relational Database Management System

## 📌 Overview

**NITCbase** is a simplified, educational **Relational Database Management System (RDBMS)** designed to help us students understand the architecture, data structures, and internal mechanics of a real-world database system.

The project provides a guided roadmap for implementing each major subsystem of an RDBMS using C++. By the end of the implementation, we will have a working database engine capable of handling basic SQL-like queries and B+ tree indexing.

---

## 🧱 Architecture

NITCbase is organized into **eight layers**, each representing a core component of the RDBMS. we will progressively implement the logic across these layers:

### 🧩 Layers (Bottom to Top)

1. **Physical Layer** – Handles raw disk read/write operations (Provided).
2. **Buffer Layer** – Implements disk block caching using LRU policy.
3. **B+ Tree Layer** – Supports creation and search operations using B+ Trees for indexing.
4. **Block Access Layer** – Manages low-level record-level operations like insert, select, and project.
5. **Cache Layer** – Manages in-memory metadata for open relations and attributes.
6. **Schema Layer** – Implements DDL commands (CREATE, DROP, RENAME tables).
7. **Algebra Layer** – Implements DML commands (SELECT, JOIN, PROJECT).
8. **Frontend Interface** – Parses SQL-like commands and routes them appropriately (Partially provided).

---

## 🎮 Interfaces

### 🧑‍💻 Frontend Interface

- Command-line interface for executing SQL-like commands.
- Translates user queries into backend operations using the Algebra and Schema layers.
- **Partially implemented and provided.**

### 💾 XFS Interface

- A low-level utility for interacting with the simulated disk system.
- Includes tools to:
  - Format the disk into NITCbase format
  - Transfer files between host and database
  - Perform low-level disk operations
- **Fully implemented and provided.**

---

## 📂 What's on the Disk?

- **Record Blocks**: Store actual tuples of database tables.
- **Index Blocks**: Store B+ Tree nodes for fast lookup.
- **Metadata Blocks**: Maintain catalogs for relations, attributes, and disk block usage.

---

## 📦 Features Supported

✅ Fixed data types: `NUMBER` and `STRING` (both 16 bytes)  
✅ Simulated disk of size 16 MB (8192 blocks of 2048 bytes)  
✅ Support for SQL-like commands such as:
- `CREATE`, `DROP`, `RENAME TABLE`
- `INSERT INTO`
- `SELECT`,`JOIN`
- `CREATE INDEX`, `DROP INDEX`
