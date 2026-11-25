// Ghidra headless post-script: decompile EVERY function in the program into a
// single directory (one file per function, named <va>.c) plus an index file
// listing "<va> <name> <size> <callee VAs...>". Used to grep the whole binary
// while reconstructing gameplay; output is not checked in (it is large).
//
// Run with:
//   export JAVA_HOME=/opt/homebrew/opt/openjdk@25/libexec/openjdk.jdk/Contents/Home
//   analyzeHeadless ghidra_project MMU -process -noanalysis -readOnly \
//     -scriptPath tools -postScript DecompileAll.java <out_dir> [start_va|0] [tools/register_signatures.txt] [end_va]

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.listing.ParameterImpl;
import ghidra.program.model.listing.Function.FunctionUpdateType;
import ghidra.program.model.data.IntegerDataType;
import ghidra.program.model.data.Undefined4DataType;
import ghidra.program.model.data.VoidDataType;
import ghidra.program.model.symbol.SourceType;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.pcode.HighFunctionDBUtil;
import ghidra.program.model.symbol.FlowType;
import ghidra.program.model.symbol.Reference;
import java.util.ArrayList;
import java.util.List;
import ghidra.util.task.ConsoleTaskMonitor;

import java.io.File;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;

public class DecompileAll extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        String out = args.length > 0 ? args[0] : "decomp_all";
        // Optional second argument: resume from this VA (hex), appending to index.txt.
        long start = args.length > 1 ? Long.parseLong(args[1].replace("0x", ""), 16) : 0;
        // Optional fourth argument: stop before this VA (hex).
        long end = args.length > 3 ? Long.parseLong(args[3].replace("0x", ""), 16) : Long.MAX_VALUE;
        File outDir = new File(out);
        outDir.mkdirs();
        // Optional third argument: register signature file (tools/register_signatures.txt).
        // Each "<va> EAX,EDI" line appends register-storage parameters so calls
        // show the values MSVC's whole-program optimiser passes in registers.
        repairFalseNoReturns();
        if (args.length > 2) applyRegisterSignatures(new File(args[2]));
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        ConsoleTaskMonitor monitor = new ConsoleTaskMonitor();
        try (PrintWriter index = new PrintWriter(new java.io.FileWriter(new File(outDir, "index.txt"), start != 0))) {
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext()) {
                Function f = it.next();
                if (f.isThunk() || f.isExternal()) continue;
                if (f.getEntryPoint().getOffset() < start) continue;
                if (f.getEntryPoint().getOffset() >= end) continue;
                String va = "0x" + Long.toHexString(f.getEntryPoint().getOffset());
                StringBuilder line = new StringBuilder(va + " " + f.getName() + " " + f.getBody().getNumAddresses());
                for (Function c : f.getCalledFunctions(monitor)) {
                    line.append(" ").append("0x").append(Long.toHexString(c.getEntryPoint().getOffset()));
                }
                index.println(line);
                DecompileResults r = decomp.decompileFunction(f, 60, monitor);
                if (!r.decompileCompleted() || r.getDecompiledFunction() == null) continue;
                String c = "// " + f.getName() + " @ " + va + "\n" + r.getDecompiledFunction().getC();
                Files.write(Paths.get(outDir.getPath(), va + ".c"), c.getBytes());
            }
        }
        decomp.dispose();
    }

    // Auto-analysis marked some ordinary helpers (e.g. the std::list iterator
    // increment at 0x42f830) as non-returning, which truncates every caller's
    // control flow. Clear the flag on any "no return" function that contains a
    // RET, re-disassemble the call fall-throughs and regrow the caller bodies.
    private void repairFalseNoReturns() throws Exception {
        List<Function> fixed = new ArrayList<>();
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext()) {
            Function f = it.next();
            if (!f.hasNoReturn() || f.isExternal() || f.isThunk()) continue;
            boolean hasRet = false;
            InstructionIterator ins = currentProgram.getListing().getInstructions(f.getBody(), true);
            while (ins.hasNext()) {
                if (ins.next().getFlowType().isTerminal()) { hasRet = true; break; }
            }
            if (!hasRet) continue;
            f.setNoReturn(false);
            fixed.add(f);
        }
        java.util.Set<Function> callers = new java.util.HashSet<>();
        for (Function f : fixed) {
            for (Reference ref : getReferencesTo(f.getEntryPoint())) {
                if (!ref.getReferenceType().isCall()) continue;
                Instruction call = getInstructionAt(ref.getFromAddress());
                if (call == null) continue;
                call.setFlowOverride(ghidra.program.model.listing.FlowOverride.NONE);
                Address next = call.getFallThrough();
                if (next == null) next = call.getMaxAddress().add(1);
                if (getInstructionAt(next) == null) {
                    new DisassembleCommand(next, null, true).applyTo(currentProgram, monitor);
                }
                Function caller = getFunctionContaining(call.getAddress());
                if (caller != null) callers.add(caller);
            }
        }
        for (Function c : callers) CreateFunctionCmd.fixupFunctionBody(currentProgram, c, monitor);
        println("repaired false no-return functions: " + fixed.size() + " callers regrown: " + callers.size());
    }

    private void applyRegisterSignatures(File file) throws Exception {
        int applied = 0;
        DecompInterface pre = new DecompInterface();
        pre.openProgram(currentProgram);
        for (String line : Files.readAllLines(file.toPath())) {
            line = line.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            String[] parts = line.split("\\s+");
            Function f = getFunctionAt(toAddr(parts[0]));
            if (f == null) continue;
            // Commit the decompiler's inferred stack parameters first; they
            // only exist at decompile time and would otherwise be dropped.
            DecompileResults dr = pre.decompileFunction(f, 60, monitor);
            if (dr.decompileCompleted() && dr.getHighFunction() != null) {
                try {
                    HighFunctionDBUtil.commitParamsToDatabase(dr.getHighFunction(), true,
                            HighFunctionDBUtil.ReturnCommitOption.COMMIT, SourceType.ANALYSIS);
                } catch (Exception e) {
                    println("commit failed " + parts[0] + ": " + e.getMessage());
                }
            }
            List<ParameterImpl> params = new ArrayList<>();
            for (Parameter p : f.getParameters()) {
                params.add(new ParameterImpl(p.getName(), p.getDataType(), p.getVariableStorage(), currentProgram));
            }
            for (String reg : parts[1].split(",")) {
                params.add(new ParameterImpl("r" + reg, IntegerDataType.dataType,
                        currentProgram.getRegister(reg), currentProgram));
            }
            Parameter ret = f.getReturn();
            ParameterImpl newRet = ret.getDataType() instanceof VoidDataType
                ? new ParameterImpl("<RETURN>", Undefined4DataType.dataType, currentProgram.getRegister("EAX"), currentProgram)
                : new ParameterImpl("<RETURN>", ret.getDataType(), ret.getVariableStorage(), currentProgram);
            try {
                f.updateFunction(f.getCallingConventionName(), newRet, params,
                        FunctionUpdateType.CUSTOM_STORAGE, true, SourceType.USER_DEFINED);
                applied++;
            } catch (Exception e) {
                println("signature failed " + parts[0] + ": " + e.getMessage());
            }
        }
        pre.dispose();
        println("applied register signatures: " + applied);
    }
}
