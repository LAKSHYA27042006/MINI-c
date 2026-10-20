// MiniC Compiler Studio Client Controller

const DEFAULT_CODE = `// MiniC Primary Academic Demonstration
int x = 3 + 4;
int y = x + 0;

while (y < 20) {
    y = y + 1;
}
`;

const SAMPLE_PROGRAMS = {
  "basic.mc": DEFAULT_CODE,
  "arithmetic.mc": `// Arithmetic & Operator Precedence Demonstration
int a = 10;
int b = 20;
int c = 5;

// Must parse as: a + (b * c)
int res1 = a + b * c;

// Must parse as: (a + b) * c
int res2 = (a + b) * c;

// Nested expression with division and modulo
int res3 = (a + b) / (c - 2) % 4;
`,
  "control_flow.mc": `// Control Flow & Scoping Demonstration
int x = 15;
int y = 0;

if (x > 10) {
    int multiplier = 3;
    y = x * multiplier;
} else {
    y = 0;
}

while (y > 10) {
    y = y - 5;
}
`,
  "optimization.mc": `// MiniC Peephole Optimization Showcase
// Rule 1: Constant Folding
int c1 = 3 + 4;
int c2 = 10 * 5;

// Rule 2: Algebraic Simplification
int a = 25;
int s1 = a + 0;
int s2 = a * 1;
int s3 = a * 0;
int s4 = a - 0;

// Rule 3: Constant Propagation leading to further folding
int p1 = 10;
int p2 = p1 + 5;

// Rule 5: Dead code (first assignment is overwritten)
int dead = 100;
dead = 200;
`,
  "nested_expression.mc": `// Nested Expressions and Float Support
float pi = 3.14159;
float radius = 5.5;
float area = pi * (radius * radius);

int complex_calc = (2 + 3) * (4 + (5 * 2)) - 10 / 2;
`,
  "errors.mc": `// Diagnostic Error Handling Demonstration
int valid = 10;
undeclared_target = valid + 5;
`
};

// DOM References
const codeEditor = document.getElementById("codeEditor");
const lineNumbers = document.getElementById("lineNumbers");
const editorStats = document.getElementById("editorStats");
const compilerStatus = document.getElementById("compilerStatus");
const exampleSelect = document.getElementById("exampleSelect");
const btnCompile = document.getElementById("btnCompile");
const btnReset = document.getElementById("btnReset");

// Tab References
const tabButtons = document.querySelectorAll(".tab-btn");
const tabContents = document.querySelectorAll(".tab-content");

// Badges
const tokenBadge = document.getElementById("tokenBadge");
const symBadge = document.getElementById("symBadge");
const errBadge = document.getElementById("errBadge");

// Metric Elements
const metricTime = document.getElementById("metricTime");
const metricOrig = document.getElementById("metricOrig");
const metricOpt = document.getElementById("metricOpt");
const metricRed = document.getElementById("metricRed");
const pipelineSummaryText = document.getElementById("pipelineSummaryText");

// Initialize Editor
function initEditor() {
  codeEditor.value = DEFAULT_CODE;
  updateLineNumbers();
  updateEditorStats();

  codeEditor.addEventListener("input", () => {
    updateLineNumbers();
    updateEditorStats();
  });

  codeEditor.addEventListener("scroll", () => {
    lineNumbers.scrollTop = codeEditor.scrollTop;
  });

  codeEditor.addEventListener("keydown", (e) => {
    if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
      e.preventDefault();
      compileSource();
    }
  });

  exampleSelect.addEventListener("change", (e) => {
    const key = e.target.value;
    if (SAMPLE_PROGRAMS[key]) {
      codeEditor.value = SAMPLE_PROGRAMS[key];
      updateLineNumbers();
      updateEditorStats();
      compileSource();
    }
  });

  btnReset.addEventListener("click", () => {
    codeEditor.value = DEFAULT_CODE;
    exampleSelect.value = "basic.mc";
    updateLineNumbers();
    updateEditorStats();
    compileSource();
  });

  btnCompile.addEventListener("click", compileSource);

  // Tabs
  tabButtons.forEach((btn) => {
    btn.addEventListener("click", () => {
      tabButtons.forEach((b) => b.classList.remove("active"));
      tabContents.forEach((c) => c.classList.remove("active"));

      btn.classList.add("active");
      const targetId = btn.getAttribute("data-tab");
      document.getElementById(targetId).classList.add("active");
    });
  });

  // AST Visual vs Text Toggle
  initASTToggle();

  // Try to load examples from server
  fetch("/api/examples")
    .then((res) => res.json())
    .then((data) => {
      if (data && Object.keys(data).length > 0) {
        Object.assign(SAMPLE_PROGRAMS, data);
      }
    })
    .catch(() => {});

  // Initial Compile
  compileSource();
}

function updateLineNumbers() {
  const lines = codeEditor.value.split("\n").length;
  let nums = "";
  for (let i = 1; i <= lines; i++) {
    nums += i + "\n";
  }
  lineNumbers.textContent = nums;
}

function updateEditorStats() {
  const lines = codeEditor.value.split("\n").length;
  const chars = codeEditor.value.length;
  editorStats.textContent = `Lines: ${lines} | Chars: ${chars}`;
}

// Compile API Call
async function compileSource() {
  const source = codeEditor.value;
  compilerStatus.textContent = "● Compiling...";
  compilerStatus.style.color = "var(--accent-amber)";

  try {
    const res = await fetch("/api/compile", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: jsonStringify({ source: source })
    });

    if (!res.ok) {
      throw new Error(`Server returned HTTP ${res.status}`);
    }

    const data = await res.json();
    renderCompilerResults(data);
  } catch (err) {
    console.warn("Backend server not reached. Rendering offline demo simulation:", err);
    renderOfflineSimulation(source);
  }
}

function jsonStringify(obj) {
  return JSON.stringify(obj);
}

// Render Results
function renderCompilerResults(data) {
  compilerStatus.textContent = data.success ? "● Success" : "● Errors Found";
  compilerStatus.style.color = data.success ? "var(--accent-green)" : "var(--accent-red)";

  // Stepper UI
  const steps = ["stepLexer", "stepParser", "stepSemantic", "stepTAC", "stepOpt", "stepReport"];
  steps.forEach((s) => {
    const el = document.getElementById(s);
    el.classList.remove("error");
    el.classList.add("completed");
  });

  if (!data.success) {
    // If errors, mark appropriate step
    const errType = (data.errors && data.errors[0]) ? data.errors[0].type : "";
    if (errType.includes("Lexical")) {
      document.getElementById("stepLexer").classList.add("error");
    } else if (errType.includes("Syntax")) {
      document.getElementById("stepParser").classList.add("error");
    } else {
      document.getElementById("stepSemantic").classList.add("error");
    }
  }

  // Metrics
  metricTime.textContent = (data.elapsed_ms || 2.5) + " ms";
  metricOrig.textContent = data.report.original_count || data.raw_tac.length;
  metricOpt.textContent = data.report.optimized_count || data.opt_tac.length;
  metricRed.textContent = (data.report.reduction_percentage || 0).toFixed(2) + "%";

  pipelineSummaryText.innerHTML = data.success
    ? `Compilation succeeded with <strong>0 errors</strong>. Intermediate code was transformed through 6 optimization passes, reducing instructions by <strong>${(data.report.reduction_percentage || 0).toFixed(2)}%</strong>.`
    : `<span style="color: var(--accent-red);">Compilation halted due to ${data.errors.length} diagnostic error(s). Review the <strong>Diagnostics</strong> tab for details.</span>`;

  // Render Tokens
  renderTokens(data.tokens || []);

  // Render AST (both visual tree and formatted text tree)
  renderAST(data.ast_text || "");

  // Render Symbols
  renderSymbols(data.symbols || []);

  // Render TAC Side-by-Side
  renderTAC(data.raw_tac || [], data.opt_tac || []);

  // Render Optimization Report
  renderReport(data.report);

  // Render Diagnostics
  renderErrors(data.errors || []);
}

function renderTokens(tokens) {
  tokenBadge.textContent = tokens.length;
  const tbody = document.getElementById("tokensTableBody");

  if (!tokens || tokens.length === 0) {
    tbody.innerHTML = `<tr><td colspan="5" style="text-align: center; color: var(--text-muted);">No tokens generated.</td></tr>`;
    return;
  }

  let html = "";
  tokens.forEach((t, idx) => {
    let pillClass = "pill-delim";
    if (t.type.startsWith("KEYWORD")) pillClass = "pill-kw";
    else if (t.type === "IDENTIFIER") pillClass = "pill-id";
    else if (t.type.includes("LITERAL")) pillClass = "pill-num";
    else if (t.type.includes("PLUS") || t.type.includes("MINUS") || t.type.includes("MULTIPLY") || t.type.includes("DIVIDE") || t.type.includes("ASSIGN") || t.type.includes("LESS") || t.type.includes("GREATER") || t.type.includes("EQUAL")) pillClass = "pill-op";

    html += `<tr>
      <td style="color: var(--text-muted);">${idx + 1}</td>
      <td style="color: var(--text-muted);">Line ${t.line}, Col ${t.column}</td>
      <td><span class="pill-token ${pillClass}">${t.type}</span></td>
      <td style="font-weight: 600;">'${escapeHtml(t.lexeme)}'</td>
      <td style="color: var(--text-muted);">${t.value ? escapeHtml(t.value) : "-"}</td>
    </tr>`;
  });
  tbody.innerHTML = html;
}

function initASTToggle() {
  const btnBinary = document.getElementById("btnAstBinary");
  const btnVisual = document.getElementById("btnAstVisual");
  const btnText = document.getElementById("btnAstText");

  const binaryContainer = document.getElementById("astBinaryContainer");
  const visualContainer = document.getElementById("astVisualContainer");
  const textContainer = document.getElementById("astTreeViewer");

  if (!btnBinary || !btnVisual || !btnText) return;

  btnBinary.addEventListener("click", () => {
    btnBinary.classList.add("active");
    btnVisual.classList.remove("active");
    btnText.classList.remove("active");

    binaryContainer.style.display = "block";
    visualContainer.style.display = "none";
    textContainer.style.display = "none";
  });

  btnVisual.addEventListener("click", () => {
    btnVisual.classList.add("active");
    btnBinary.classList.remove("active");
    btnText.classList.remove("active");

    binaryContainer.style.display = "none";
    visualContainer.style.display = "block";
    textContainer.style.display = "none";
  });

  btnText.addEventListener("click", () => {
    btnText.classList.add("active");
    btnBinary.classList.remove("active");
    btnVisual.classList.remove("active");

    binaryContainer.style.display = "none";
    visualContainer.style.display = "none";
    textContainer.style.display = "block";
  });
}

// Convert indented text into tree structure
function parseIndentedAST(text) {
  if (!text) return null;
  const lines = text.split("\n").filter((l) => l.trim().length > 0);
  if (lines.length === 0) return null;

  const root = { label: "Program Root", depth: -1, children: [] };
  const stack = [root];

  lines.forEach((rawLine) => {
    const indentMatch = rawLine.match(/^(\s*)/);
    const leadingSpaces = indentMatch ? indentMatch[1].length : 0;
    const depth = Math.floor(leadingSpaces / 2);
    const cleanText = rawLine.trim();

    const node = { label: cleanText, depth: depth, children: [] };
    while (stack.length > 1 && stack[stack.length - 1].depth >= depth) {
      stack.pop();
    }
    stack[stack.length - 1].children.push(node);
    stack.push(node);
  });

  return root;
}

// Transform AST node into clean binary expression node
function cleanBinaryTreeNode(node) {
  if (!node) return null;
  const t = node.label;

  // BinaryOp ('+')
  if (t.startsWith("BinaryOp")) {
    const match = t.match(/BinaryOp \('([^']+)'\)/);
    const op = match ? match[1] : "+";
    return {
      label: op,
      type: "operator",
      tag: "OP",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // UnaryOp ('-')
  if (t.startsWith("UnaryOp")) {
    const match = t.match(/UnaryOp \('([^']+)'\)/);
    const op = match ? match[1] : "-";
    return {
      label: op,
      type: "operator",
      tag: "UNARY",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // Identifier: 'a'
  if (t.startsWith("Identifier:")) {
    const match = t.match(/Identifier: '([^']+)'/);
    const id = match ? match[1] : "id";
    return {
      label: id,
      type: "identifier",
      tag: "ID",
      children: []
    };
  }

  // IntLiteral: 10
  if (t.startsWith("IntLiteral:")) {
    const val = t.replace("IntLiteral:", "").trim();
    return {
      label: val,
      type: "literal",
      tag: "INT",
      children: []
    };
  }

  // FloatLiteral: 3.14
  if (t.startsWith("FloatLiteral:")) {
    const val = t.replace("FloatLiteral:", "").trim();
    return {
      label: val,
      type: "literal",
      tag: "FLOAT",
      children: []
    };
  }

  // InitExpr: pass-through to expression child
  if (t.startsWith("InitExpr:")) {
    if (node.children.length === 1) {
      return cleanBinaryTreeNode(node.children[0]);
    }
    return {
      label: "init",
      type: "branch",
      tag: "INIT",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // Condition: pass-through to expression child
  if (t.startsWith("Condition:")) {
    if (node.children.length === 1) {
      return cleanBinaryTreeNode(node.children[0]);
    }
    return {
      label: "cond",
      type: "branch",
      tag: "COND",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // VarDecl: int a [Line 10]
  if (t.startsWith("VarDecl:")) {
    const namePart = t.replace("VarDecl:", "").split("[")[0].trim();
    return {
      label: namePart,
      type: "vardecl",
      tag: "DECL",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // Assignment: y = [Line 14]
  if (t.startsWith("Assignment:")) {
    const namePart = t.replace("Assignment:", "").split("[")[0].trim();
    return {
      label: namePart,
      type: "assign",
      tag: "ASSIGN",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // WhileStatement
  if (t.startsWith("WhileStatement")) {
    return {
      label: "while ( )",
      type: "control",
      tag: "WHILE",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // IfStatement
  if (t.startsWith("IfStatement")) {
    return {
      label: "if ( )",
      type: "control",
      tag: "IF",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // Block
  if (t.startsWith("Block")) {
    return {
      label: "{ body }",
      type: "control",
      tag: "BLOCK",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  // Body / ThenBranch / ElseBranch
  if (t.startsWith("Body:") || t.startsWith("ThenBranch:") || t.startsWith("ElseBranch:")) {
    if (node.children.length === 1) {
      return cleanBinaryTreeNode(node.children[0]);
    }
    return {
      label: t.replace(":", ""),
      type: "branch",
      tag: "BRANCH",
      children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
    };
  }

  return {
    label: t,
    type: "generic",
    tag: "NODE",
    children: node.children.map(cleanBinaryTreeNode).filter(Boolean)
  };
}

// Generate HTML <ul> <li> tree structure
function generateBinaryTreeHTML(node) {
  if (!node) return "";
  let badgeClass = "node-id";

  if (node.type === "operator") badgeClass = "node-op";
  else if (node.type === "identifier") badgeClass = "node-id";
  else if (node.type === "literal") badgeClass = "node-num";
  else if (node.type === "vardecl" || node.type === "assign") badgeClass = "node-stmt";
  else if (node.type === "control") badgeClass = "node-ctrl";

  let childrenHTML = "";
  if (node.children && node.children.length > 0) {
    childrenHTML = "<ul>" + node.children.map(generateBinaryTreeHTML).join("") + "</ul>";
  }

  return `
    <li>
      <div class="tree-node ${badgeClass}">
        <span class="node-tag">${escapeHtml(node.tag || '')}</span>
        <span>${escapeHtml(node.label)}</span>
      </div>
      ${childrenHTML}
    </li>
  `;
}

// Store current parsed tree for subtree selector
let currentParsedAST = null;

function renderAST(astText) {
  const textViewer = document.getElementById("astTreeViewer");
  const visualContainer = document.getElementById("astVisualContainer");
  const binaryRoot = document.getElementById("binaryTreeRoot");
  const nodeCountBadge = document.getElementById("astNodeCount");
  const subtreeSelect = document.getElementById("astSubtreeSelect");

  textViewer.textContent = astText || "(No AST generated)";

  if (!astText || !astText.trim()) {
    visualContainer.innerHTML = `<span style="color: var(--text-muted);">(No AST generated)</span>`;
    binaryRoot.innerHTML = `<span style="color: var(--text-muted);">(No AST generated)</span>`;
    if (nodeCountBadge) nodeCountBadge.textContent = "0 Nodes";
    return;
  }

  const lines = astText.split("\n").filter((l) => l.trim().length > 0);
  if (nodeCountBadge) nodeCountBadge.textContent = `${lines.length} Nodes`;

  // 1. Build and Render Outline Hierarchy View
  let outlineHtml = "";
  lines.forEach((rawLine) => {
    const indentMatch = rawLine.match(/^(\s*)/);
    const leadingSpaces = indentMatch ? indentMatch[1].length : 0;
    const depth = Math.floor(leadingSpaces / 2);
    const text = rawLine.trim();

    let badgeClass = "ast-badge-branch";
    let badgeLabel = "NODE";
    let icon = "📂";
    let label = text;
    let meta = "";

    if (text.startsWith("Program")) {
      badgeClass = "ast-badge-root";
      badgeLabel = "PROGRAM";
      icon = "📦";
      label = "Program Root";
      meta = text.replace("Program", "").trim();
    } else if (text.startsWith("VarDecl:")) {
      badgeClass = "ast-badge-stmt";
      badgeLabel = "VAR_DECL";
      icon = "🏷️";
      label = text.replace("VarDecl:", "").trim();
    } else if (text.startsWith("Assignment:")) {
      badgeClass = "ast-badge-stmt";
      badgeLabel = "ASSIGN";
      icon = "✏️";
      label = text.replace("Assignment:", "").trim();
    } else if (text.startsWith("BinaryOp")) {
      badgeClass = "ast-badge-op";
      badgeLabel = "BINARY_OP";
      icon = "⚡";
      label = text;
    } else if (text.startsWith("UnaryOp")) {
      badgeClass = "ast-badge-op";
      badgeLabel = "UNARY_OP";
      icon = "⚡";
      label = text;
    } else if (text.startsWith("IntLiteral:")) {
      badgeClass = "ast-badge-literal";
      badgeLabel = "INT";
      icon = "🔢";
      label = text.replace("IntLiteral:", "").trim();
    } else if (text.startsWith("FloatLiteral:")) {
      badgeClass = "ast-badge-literal";
      badgeLabel = "FLOAT";
      icon = "🔢";
      label = text.replace("FloatLiteral:", "").trim();
    } else if (text.startsWith("Identifier:")) {
      badgeClass = "ast-badge-ident";
      badgeLabel = "IDENT";
      icon = "🆔";
      label = text.replace("Identifier:", "").trim();
    } else if (text.startsWith("WhileStatement")) {
      badgeClass = "ast-badge-ctrl";
      badgeLabel = "WHILE";
      icon = "🔄";
      label = "While Loop";
      meta = text.replace("WhileStatement", "").trim();
    } else if (text.startsWith("IfStatement")) {
      badgeClass = "ast-badge-ctrl";
      badgeLabel = "IF";
      icon = "🔀";
      label = "If Statement";
      meta = text.replace("IfStatement", "").trim();
    } else if (text.startsWith("Block")) {
      badgeClass = "ast-badge-stmt";
      badgeLabel = "BLOCK";
      icon = "🧱";
      label = "Scope Block";
      meta = text.replace("Block", "").trim();
    } else if (text.startsWith("InitExpr:")) {
      badgeClass = "ast-badge-branch";
      badgeLabel = "INIT";
      icon = "↳";
      label = "Initialization Expression";
    } else if (text.startsWith("Condition:")) {
      badgeClass = "ast-badge-branch";
      badgeLabel = "COND";
      icon = "❓";
      label = "Condition Expression";
    } else if (text.startsWith("Body:")) {
      badgeClass = "ast-badge-branch";
      badgeLabel = "BODY";
      icon = "▶";
      label = "Loop Body";
    }

    let guideStr = "";
    for (let d = 0; d < depth; d++) {
      guideStr += "│   ";
    }
    const connector = depth > 0 ? "├── " : "";

    outlineHtml += `
      <div class="ast-row">
        <span class="ast-indent-guide">${guideStr}</span>
        ${connector ? `<span class="ast-connector">${connector}</span>` : ""}
        <div class="ast-node-card">
          <span class="ast-badge ${badgeClass}">${icon} ${badgeLabel}</span>
          <span class="ast-node-label">${escapeHtml(label)}</span>
          ${meta ? `<span class="ast-node-meta">${escapeHtml(meta)}</span>` : ""}
        </div>
      </div>
    `;
  });
  visualContainer.innerHTML = outlineHtml;

  // 2. Parse Hierarchical Tree Object for Graphical Binary Tree View
  const parsedRoot = parseIndentedAST(astText);
  currentParsedAST = parsedRoot;

  // Populate subtree selector
  if (subtreeSelect && parsedRoot && parsedRoot.children.length > 0) {
    const progNode = parsedRoot.children[0];
    const stmts = progNode.children || [];

    let selectHtml = `<option value="all">Full Program Tree (${stmts.length} statements)</option>`;
    stmts.forEach((stmt, idx) => {
      const summary = stmt.label.split("[")[0].trim();
      selectHtml += `<option value="${idx}">Stmt ${idx + 1}: ${escapeHtml(summary)}</option>`;
    });
    subtreeSelect.innerHTML = selectHtml;

    subtreeSelect.onchange = (e) => {
      renderSelectedSubtree(e.target.value);
    };
  }

  // Render initial binary tree view (Full Program or first expression)
  renderSelectedSubtree("all");
}

function renderSelectedSubtree(val) {
  const binaryRoot = document.getElementById("binaryTreeRoot");
  if (!binaryRoot || !currentParsedAST || !currentParsedAST.children.length) return;

  const progNode = currentParsedAST.children[0];
  const stmts = progNode.children || [];

  let targetNode = null;
  if (val === "all") {
    // Show Program Root with each statement as child
    targetNode = {
      label: "Program",
      type: "control",
      tag: "ROOT",
      children: stmts.map(cleanBinaryTreeNode).filter(Boolean)
    };
  } else {
    const idx = parseInt(val, 10);
    if (!isNaN(idx) && stmts[idx]) {
      targetNode = cleanBinaryTreeNode(stmts[idx]);
    }
  }

  if (targetNode) {
    binaryRoot.innerHTML = `<ul>${generateBinaryTreeHTML(targetNode)}</ul>`;
  }
}

function renderSymbols(symbols) {
  symBadge.textContent = symbols.length;
  const tbody = document.getElementById("symTableBody");

  if (!symbols || symbols.length === 0) {
    tbody.innerHTML = `<tr><td colspan="6" style="text-align: center; color: var(--text-muted);">No symbols recorded.</td></tr>`;
    return;
  }

  let html = "";
  symbols.forEach((s) => {
    html += `<tr>
      <td style="font-weight: 700; color: #58a6ff;">${escapeHtml(s.name)}</td>
      <td><span class="pill-token pill-kw">${escapeHtml(s.type)}</span></td>
      <td><span class="pill-token pill-id">${escapeHtml(s.scope)}</span></td>
      <td>Line ${s.line}</td>
      <td style="color: ${s.initialized === 'Yes' ? 'var(--accent-green)' : 'var(--text-muted)'};">${s.initialized}</td>
      <td>${escapeHtml(s.const_val || '-')}</td>
    </tr>`;
  });
  tbody.innerHTML = html;
}

function renderTAC(rawTac, optTac) {
  document.getElementById("rawTacCount").textContent = `${rawTac.length} instructions`;
  document.getElementById("optTacCount").textContent = `${optTac.length} instructions`;

  document.getElementById("rawTacViewer").textContent = rawTac.join("\n") || "// No raw TAC generated";
  document.getElementById("optTacViewer").textContent = optTac.join("\n") || "// No optimized TAC generated";
}

function renderReport(report) {
  if (!report) return;

  document.getElementById("repOrig").textContent = report.original_count;
  document.getElementById("repOpt").textContent = report.optimized_count;
  document.getElementById("repRem").textContent = report.instructions_removed;
  document.getElementById("repPct").textContent = (report.reduction_percentage || 0).toFixed(2) + "%";

  const totalTrans = Math.max(
    1,
    (report.constant_folding || 0) +
    (report.algebraic_simplification || 0) +
    (report.constant_propagation || 0) +
    (report.temp_elimination || 0) +
    (report.dead_code || 0) +
    (report.unreachable_code || 0)
  );

  updatePassBar("barFolding", "countFolding", report.constant_folding || 0, totalTrans);
  updatePassBar("barAlg", "countAlg", report.algebraic_simplification || 0, totalTrans);
  updatePassBar("barProp", "countProp", report.constant_propagation || 0, totalTrans);
  updatePassBar("barTemp", "countTemp", report.temp_elimination || 0, totalTrans);
  updatePassBar("barDead", "countDead", report.dead_code || 0, totalTrans);
  updatePassBar("barUnreach", "countUnreach", report.unreachable_code || 0, totalTrans);
}

function updatePassBar(barId, countId, count, total) {
  document.getElementById(countId).textContent = count;
  const pct = Math.min(100, Math.round((count / total) * 100));
  document.getElementById(barId).style.width = `${pct}%`;
}

function renderErrors(errors) {
  const container = document.getElementById("errorContainer");
  if (!errors || errors.length === 0) {
    errBadge.style.display = "none";
    container.innerHTML = `<p style="color: var(--accent-green); font-size: 13px;">✔ No compilation errors. Zero diagnostics reported.</p>`;
    return;
  }

  errBadge.style.display = "inline-block";
  errBadge.textContent = errors.length;

  let html = "";
  errors.forEach((err) => {
    html += `<div class="error-card">
      <div class="error-title">
        <span>⚠</span> ${escapeHtml(err.type || 'Error')}
        ${err.line ? `<span style="font-weight: 400; font-size: 12px; color: var(--text-muted);">(Line ${err.line}, Column ${err.column})</span>` : ''}
      </div>
      <div class="error-desc">${escapeHtml(err.message || err.raw)}</div>
    </div>`;
  });
  container.innerHTML = html;
}

function escapeHtml(str) {
  if (!str) return "";
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

// Fallback offline simulation when server.py is not yet started
function renderOfflineSimulation(source) {
  compilerStatus.textContent = "● Offline Preview";
  compilerStatus.style.color = "var(--accent-blue)";

  // Check if error sample
  if (source.includes("undeclared_target")) {
    renderCompilerResults({
      success: false,
      elapsed_ms: 1.8,
      tokens: [
        { line: 2, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
        { line: 2, column: 5, type: "IDENTIFIER", lexeme: "valid", value: "" },
        { line: 2, column: 11, type: "INTEGER_LITERAL", lexeme: "10", value: "10" },
        { line: 3, column: 1, type: "IDENTIFIER", lexeme: "undeclared_target", value: "" }
      ],
      ast_text: "Program (statements: 1)\n  VarDecl: int valid [Line 2]\n    InitExpr: IntLiteral: 10",
      symbols: [{ name: "valid", type: "int", scope: "global", line: "2", initialized: "Yes", const_val: "-" }],
      raw_tac: ["    valid  = 10"],
      opt_tac: ["    valid  = 10"],
      report: { original_count: 1, optimized_count: 1, instructions_removed: 0, reduction_percentage: 0 },
      errors: [{ type: "Semantic Error", line: 3, column: 1, message: "Assignment to undeclared variable 'undeclared_target'." }]
    });
    return;
  }

  // Check if arithmetic.mc sample
  if (source.includes("res1 = a + b * c")) {
    renderCompilerResults({
      success: true,
      elapsed_ms: 2.1,
      tokens: [
        { line: 10, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
        { line: 10, column: 5, type: "IDENTIFIER", lexeme: "a", value: "" },
        { line: 10, column: 9, type: "INTEGER_LITERAL", lexeme: "10", value: "10" },
        { line: 11, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
        { line: 11, column: 5, type: "IDENTIFIER", lexeme: "b", value: "" },
        { line: 11, column: 9, type: "INTEGER_LITERAL", lexeme: "20", value: "20" },
        { line: 12, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
        { line: 12, column: 5, type: "IDENTIFIER", lexeme: "c", value: "" },
        { line: 12, column: 9, type: "INTEGER_LITERAL", lexeme: "5", value: "5" },
        { line: 15, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
        { line: 15, column: 5, type: "IDENTIFIER", lexeme: "res1", value: "" },
        { line: 15, column: 14, type: "PLUS", lexeme: "+", value: "" },
        { line: 15, column: 18, type: "MULTIPLY", lexeme: "*", value: "" }
      ],
      ast_text: `Program (statements: 6)
  VarDecl: int a [Line 10]
    InitExpr:
      IntLiteral: 10
  VarDecl: int b [Line 11]
    InitExpr:
      IntLiteral: 20
  VarDecl: int c [Line 12]
    InitExpr:
      IntLiteral: 5
  VarDecl: int res1 [Line 15]
    InitExpr:
      BinaryOp ('+') [Type: int]
        Identifier: 'a' [Type: int]
        BinaryOp ('*') [Type: int]
          Identifier: 'b' [Type: int]
          Identifier: 'c' [Type: int]
  VarDecl: int res2 [Line 18]
    InitExpr:
      BinaryOp ('*') [Type: int]
        BinaryOp ('+') [Type: int]
          Identifier: 'a' [Type: int]
          Identifier: 'b' [Type: int]
        Identifier: 'c' [Type: int]
  VarDecl: int res3 [Line 21]
    InitExpr:
      BinaryOp ('%') [Type: int]
        BinaryOp ('/') [Type: int]
          BinaryOp ('+') [Type: int]
            Identifier: 'a' [Type: int]
            Identifier: 'b' [Type: int]
          BinaryOp ('-') [Type: int]
            Identifier: 'c' [Type: int]
            IntLiteral: 2
        IntLiteral: 4`,
      symbols: [
        { name: "a", type: "int", scope: "global", line: "10", initialized: "Yes", const_val: "-" },
        { name: "b", type: "int", scope: "global", line: "11", initialized: "Yes", const_val: "-" },
        { name: "c", type: "int", scope: "global", line: "12", initialized: "Yes", const_val: "-" },
        { name: "res1", type: "int", scope: "global", line: "15", initialized: "Yes", const_val: "-" },
        { name: "res2", type: "int", scope: "global", line: "18", initialized: "Yes", const_val: "-" },
        { name: "res3", type: "int", scope: "global", line: "21", initialized: "Yes", const_val: "-" }
      ],
      raw_tac: [
        "    a      = 10",
        "    b      = 20",
        "    c      = 5",
        "    t1     = b * c",
        "    t2     = a + t1",
        "    res1   = t2",
        "    t3     = a + b",
        "    t4     = t3 * c",
        "    res2   = t4",
        "    t5     = a + b",
        "    t6     = c - 2",
        "    t7     = t5 / t6",
        "    t8     = t7 % 4",
        "    res3   = t8"
      ],
      opt_tac: [
        "    a      = 10",
        "    b      = 20",
        "    c      = 5",
        "    t1     = 100",
        "    res1   = 10 + t1",
        "    t3     = 30",
        "    res2   = t3 * 5",
        "    t5     = 30",
        "    t6     = 3",
        "    t7     = t5 / t6",
        "    res3   = t7 % 4"
      ],
      report: {
        original_count: 14,
        optimized_count: 11,
        instructions_removed: 3,
        constant_folding: 4,
        algebraic_simplification: 0,
        constant_propagation: 9,
        temp_elimination: 3,
        dead_code: 0,
        unreachable_code: 0,
        reduction_percentage: 21.43
      },
      errors: []
    });
    return;
  }

  // Default basic.mc simulation
  renderCompilerResults({
    success: true,
    elapsed_ms: 2.4,
    tokens: [
      { line: 2, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
      { line: 2, column: 5, type: "IDENTIFIER", lexeme: "x", value: "" },
      { line: 2, column: 7, type: "ASSIGN", lexeme: "=", value: "" },
      { line: 2, column: 9, type: "INTEGER_LITERAL", lexeme: "3", value: "3" },
      { line: 2, column: 11, type: "PLUS", lexeme: "+", value: "" },
      { line: 2, column: 13, type: "INTEGER_LITERAL", lexeme: "4", value: "4" },
      { line: 2, column: 14, type: "SEMICOLON", lexeme: ";", value: "" },
      { line: 3, column: 1, type: "KEYWORD_INT", lexeme: "int", value: "" },
      { line: 3, column: 5, type: "IDENTIFIER", lexeme: "y", value: "" },
      { line: 3, column: 7, type: "ASSIGN", lexeme: "=", value: "" },
      { line: 3, column: 9, type: "IDENTIFIER", lexeme: "x", value: "" },
      { line: 3, column: 11, type: "PLUS", lexeme: "+", value: "" },
      { line: 3, column: 13, type: "INTEGER_LITERAL", lexeme: "0", value: "0" },
      { line: 3, column: 14, type: "SEMICOLON", lexeme: ";", value: "" },
      { line: 5, column: 1, type: "KEYWORD_WHILE", lexeme: "while", value: "" },
      { line: 5, column: 7, type: "LEFT_PAREN", lexeme: "(", value: "" },
      { line: 5, column: 8, type: "IDENTIFIER", lexeme: "y", value: "" },
      { line: 5, column: 10, type: "LESS", lexeme: "<", value: "" },
      { line: 5, column: 12, type: "INTEGER_LITERAL", lexeme: "20", value: "20" },
      { line: 5, column: 14, type: "RIGHT_PAREN", lexeme: ")", value: "" },
      { line: 5, column: 16, type: "LEFT_BRACE", lexeme: "{", value: "" },
      { line: 6, column: 5, type: "IDENTIFIER", lexeme: "y", value: "" },
      { line: 6, column: 7, type: "ASSIGN", lexeme: "=", value: "" },
      { line: 6, column: 9, type: "IDENTIFIER", lexeme: "y", value: "" },
      { line: 6, column: 11, type: "PLUS", lexeme: "+", value: "" },
      { line: 6, column: 13, type: "INTEGER_LITERAL", lexeme: "1", value: "1" },
      { line: 6, column: 14, type: "SEMICOLON", lexeme: ";", value: "" },
      { line: 7, column: 1, type: "RIGHT_BRACE", lexeme: "}", value: "" },
      { line: 8, column: 1, type: "EOF", lexeme: "EOF", value: "" }
    ],
    ast_text: `Program (statements: 3)
  VarDecl: int x [Line 2]
    InitExpr:
      BinaryOp ('+') [Type: int]
        IntLiteral: 3
        IntLiteral: 4
  VarDecl: int y [Line 3]
    InitExpr:
      BinaryOp ('+') [Type: int]
        Identifier: 'x' [Type: int]
        IntLiteral: 0
  WhileStatement [Line 5]
    Condition:
      BinaryOp ('<') [Type: int]
        Identifier: 'y' [Type: int]
        IntLiteral: 20
    Body:
      Block (statements: 1) [Line 5]
        Assignment: y = [Line 6]
          BinaryOp ('+') [Type: int]
            Identifier: 'y' [Type: int]
            IntLiteral: 1`,
    symbols: [
      { name: "x", type: "int", scope: "global", line: "2", initialized: "Yes", const_val: "-" },
      { name: "y", type: "int", scope: "global", line: "3", initialized: "Yes", const_val: "-" }
    ],
    raw_tac: [
      "    t1     = 3 + 4",
      "    x      = t1",
      "    t2     = x + 0",
      "    y      = t2",
      "L1        :",
      "    t3     = y < 20",
      "    ifFalse t3 goto L2",
      "    t4     = y + 1",
      "    y      = t4",
      "    goto L1",
      "L2        :"
    ],
    opt_tac: [
      "    x      = 7",
      "    y      = 7",
      "L1        :",
      "    t3     = y < 20",
      "    ifFalse t3 goto L2",
      "    y      = y + 1",
      "    goto L1",
      "L2        :"
    ],
    report: {
      original_count: 11,
      optimized_count: 8,
      instructions_removed: 3,
      constant_folding: 1,
      algebraic_simplification: 1,
      constant_propagation: 1,
      temp_elimination: 3,
      dead_code: 0,
      unreachable_code: 0,
      reduction_percentage: 27.27
    },
    errors: []
  });
}

// Start app on DOM load
document.addEventListener("DOMContentLoaded", initEditor);
