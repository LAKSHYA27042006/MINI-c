import os
import sys
import json
import time
import tempfile
import subprocess
import webbrowser
from http.server import HTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse

PORT = 8080
PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
WEB_DIR = os.path.join(PROJECT_DIR, "web")
EXAMPLES_DIR = os.path.join(PROJECT_DIR, "examples")
MINIC_EXE = os.path.join(PROJECT_DIR, "minic.exe" if os.name == "nt" else "minic")


def ensure_binary_built():
    """Build minic executable if not present."""
    if not os.path.isfile(MINIC_EXE):
        print(f"[*] Executable '{MINIC_EXE}' not found. Compiling...")
        cmd = [
            "gcc",
            "-std=c11",
            "-Wall",
            "-Wextra",
            "-Wpedantic",
            "-O2",
            "-Isrc",
            "src/error.c",
            "src/token.c",
            "src/lexer.c",
            "src/ast.c",
            "src/parser.c",
            "src/symbol_table.c",
            "src/semantic.c",
            "src/tac.c",
            "src/optimizer.c",
            "src/main.c",
            "-o",
            MINIC_EXE,
        ]
        ret = subprocess.run(cmd, cwd=PROJECT_DIR)
        if ret.returncode != 0:
            print("[!] Failed to compile minic compiler.")
            sys.exit(1)
        print("[+] Compilation succeeded.")


def parse_compiler_output(stdout_text, stderr_text, return_code):
    """Parses raw text output of 'minic <file> --all' into structured JSON data."""
    result = {
        "success": return_code == 0,
        "raw_stdout": stdout_text,
        "raw_stderr": stderr_text,
        "tokens": [],
        "ast_text": "",
        "symbols": [],
        "raw_tac": [],
        "opt_tac": [],
        "report": {
            "original_count": 0,
            "optimized_count": 0,
            "instructions_removed": 0,
            "constant_folding": 0,
            "algebraic_simplification": 0,
            "constant_propagation": 0,
            "temp_elimination": 0,
            "dead_code": 0,
            "unreachable_code": 0,
            "reduction_percentage": 0.0,
        },
        "errors": [],
    }

    # Extract errors from stdout and stderr
    full_text = stdout_text + "\n" + stderr_text
    for line in full_text.splitlines():
        line_clean = line.strip()
        if line_clean.startswith("[") and ("Error" in line_clean or "Warning" in line_clean):
            # Format: [Lexical Error] Line 10, Column 5: message
            # Or: [Semantic Error] Line 29, Column 1: message
            err_type = "Error"
            line_no = 0
            col_no = 0
            msg = line_clean

            try:
                parts = line_clean.split("]", 1)
                err_type = parts[0].replace("[", "").strip()
                rest = parts[1].strip()

                if "Line " in rest:
                    coord_part, msg_part = rest.split(":", 1)
                    coords = coord_part.split(",")
                    for c in coords:
                        if "Line" in c:
                            line_no = int(c.replace("Line", "").strip())
                        elif "Column" in c:
                            col_no = int(c.replace("Column", "").strip())
                    msg = msg_part.strip()
            except Exception:
                pass

            result["errors"].append({
                "type": err_type,
                "line": line_no,
                "column": col_no,
                "message": msg,
                "raw": line_clean
            })

    # Section parsing
    # 1. Tokens Section
    if "TOKEN STREAM" in stdout_text:
        token_part = stdout_text.split("TOKEN STREAM", 1)[1]
        for tline in token_part.splitlines():
            tline = tline.strip()
            if tline.startswith("[Line"):
                try:
                    meta, rest = tline.split("]", 1)
                    meta_parts = meta.replace("[", "").split(",")
                    line_n = int(meta_parts[0].replace("Line", "").strip())
                    col_n = int(meta_parts[1].replace("Col", "").strip())

                    type_part, lex_part = rest.split(":", 1)
                    token_type = type_part.strip()

                    val_str = ""
                    lexeme = lex_part.strip()
                    if "(value:" in lexeme:
                        lexeme, val_part = lexeme.split("(value:", 1)
                        val_str = val_part.replace(")", "").strip()
                    lexeme = lexeme.strip().strip("'")

                    result["tokens"].append({
                        "line": line_n,
                        "column": col_n,
                        "type": token_type,
                        "lexeme": lexeme,
                        "value": val_str
                    })
                except Exception:
                    pass
            elif tline.startswith("========================================") and len(result["tokens"]) > 0:
                break

    # 2. AST Section
    if "ABSTRACT SYNTAX TREE (AST)" in stdout_text:
        ast_part = stdout_text.split("ABSTRACT SYNTAX TREE (AST)", 1)[1]
        ast_blocks = [p.strip() for p in ast_part.split("========================================") if p.strip()]
        if ast_blocks:
            result["ast_text"] = ast_blocks[0]

    # 3. Symbol Table Directory
    if "SYMBOL TABLE DIRECTORY" in stdout_text:
        sym_part = stdout_text.split("SYMBOL TABLE DIRECTORY", 1)[1]
        start_table = False
        for s_line in sym_part.splitlines():
            s_line = s_line.strip()
            if "----------------------------------" in s_line:
                start_table = True
                continue
            if start_table and s_line.startswith("==="):
                break
            if start_table and s_line:
                cols = s_line.split()
                if len(cols) >= 5:
                    result["symbols"].append({
                        "name": cols[0],
                        "type": cols[1],
                        "scope": cols[2],
                        "line": cols[3],
                        "initialized": cols[4],
                        "const_val": cols[5] if len(cols) > 5 else "-"
                    })

    # 4. Raw TAC Section
    if "RAW THREE-ADDRESS CODE" in stdout_text:
        tac_part = stdout_text.split("RAW THREE-ADDRESS CODE", 1)[1]
        tac_blocks = [p.strip() for p in tac_part.split("========================================") if p.strip()]
        if tac_blocks:
            result["raw_tac"] = [ln.rstrip() for ln in tac_blocks[0].splitlines() if ln.strip()]

    # 5. Optimized TAC Section
    if "OPTIMIZED THREE-ADDRESS CODE" in stdout_text:
        opt_part = stdout_text.split("OPTIMIZED THREE-ADDRESS CODE", 1)[1]
        opt_blocks = [p.strip() for p in opt_part.split("========================================") if p.strip()]
        if opt_blocks:
            result["opt_tac"] = [ln.rstrip() for ln in opt_blocks[0].splitlines() if ln.strip()]

    # 6. Optimization Report
    if "OPTIMIZATION REPORT" in stdout_text:
        rep_part = stdout_text.split("OPTIMIZATION REPORT", 1)[1]
        rep_blocks = [p.strip() for p in rep_part.split("========================================") if p.strip()]
        if rep_blocks:
            for r_line in rep_blocks[0].splitlines():
                r_line = r_line.strip()
                if ":" in r_line:
                    key, val = r_line.split(":", 1)
                    key = key.strip()
                    val = val.strip()
                    try:
                        if "Original TAC instructions" in key:
                            result["report"]["original_count"] = int(val)
                        elif "Optimized TAC instructions" in key:
                            result["report"]["optimized_count"] = int(val)
                        elif "Instructions removed" in key:
                            result["report"]["instructions_removed"] = int(val)
                        elif "Constant folding" in key:
                            result["report"]["constant_folding"] = int(val)
                        elif "Algebraic simplification" in key:
                            result["report"]["algebraic_simplification"] = int(val)
                        elif "Constant propagation" in key:
                            result["report"]["constant_propagation"] = int(val)
                        elif "Temporary elimination" in key:
                            result["report"]["temp_elimination"] = int(val)
                        elif "Dead code elimination" in key:
                            result["report"]["dead_code"] = int(val)
                        elif "Unreachable code removal" in key:
                            result["report"]["unreachable_code"] = int(val)
                        elif "Reduction" in key:
                            result["report"]["reduction_percentage"] = float(val.replace("%", "").strip())
                    except ValueError:
                        pass

    return result


class CompilerRequestHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/examples":
            self.handle_get_examples()
        elif parsed.path == "/api/health":
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(b'{"status": "ok", "compiler": "MiniC C11"}')
        else:
            super().do_GET()

    def do_POST(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/compile":
            self.handle_post_compile()
        else:
            self.send_error(404, "Endpoint not found")

    def handle_get_examples(self):
        """Returns all sample programs from examples/ folder."""
        examples = {}
        if os.path.isdir(EXAMPLES_DIR):
            for fname in sorted(os.listdir(EXAMPLES_DIR)):
                if fname.endswith(".mc"):
                    fpath = os.path.join(EXAMPLES_DIR, fname)
                    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
                        examples[fname] = f.read()

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps(examples).encode("utf-8"))

    def handle_post_compile(self):
        """Receives source code, executes minic compiler, and returns structured results."""
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length)

        try:
            req_data = json.loads(body.decode("utf-8"))
            source_code = req_data.get("source", "")
        except Exception as e:
            self.send_response(400)
            self.end_headers()
            self.wfile.write(json.dumps({"error": f"Invalid JSON payload: {e}"}).encode("utf-8"))
            return

        ensure_binary_built()

        # Write to temporary file and execute
        start_time = time.perf_counter()
        with tempfile.NamedTemporaryFile("w", suffix=".mc", delete=False, encoding="utf-8") as temp_file:
            temp_path = temp_file.name
            temp_file.write(source_code)

        try:
            cmd = [MINIC_EXE, temp_path, "--all"]
            proc = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=PROJECT_DIR,
                timeout=10,
            )
            elapsed_ms = (time.perf_counter() - start_time) * 1000.0

            result = parse_compiler_output(proc.stdout, proc.stderr, proc.returncode)
            result["elapsed_ms"] = round(elapsed_ms, 2)
            result["exit_code"] = proc.returncode

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(result).encode("utf-8"))

        except subprocess.TimeoutExpired:
            self.send_response(504)
            self.end_headers()
            self.wfile.write(json.dumps({"error": "Compilation timed out"}).encode("utf-8"))
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(json.dumps({"error": str(e)}).encode("utf-8"))
        finally:
            if os.path.exists(temp_path):
                try:
                    os.remove(temp_path)
                except Exception:
                    pass


def main():
    ensure_binary_built()
    print("================================================================")
    print("      MiniC Compiler Studio — Interactive Web Playground         ")
    print("================================================================")
    print(f"[*] Serving web frontend from: {WEB_DIR}")
    print(f"[*] MiniC compiler binary    : {MINIC_EXE}")
    print(f"[*] Local Web Studio URL     : http://localhost:{PORT}")
    print("----------------------------------------------------------------")
    print("[*] Press Ctrl+C in terminal to stop server.")
    print("================================================================\n")

    # Automatically launch browser
    try:
        webbrowser.open(f"http://localhost:{PORT}")
    except Exception:
        pass

    server = HTTPServer(("localhost", PORT), CompilerRequestHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[*] Stopping MiniC Web Studio server. Goodbye!")
        server.server_close()


if __name__ == "__main__":
    main()
