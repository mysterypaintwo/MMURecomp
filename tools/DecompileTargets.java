// Ghidra headless post-script: decompile functions listed in tools/decompile_targets.txt
// and write each result to docs/decompiled/<address>.c.
//
// Run with:
//   export JAVA_HOME=/opt/homebrew/opt/openjdk@25/libexec/openjdk.jdk/Contents/Home
//   export PATH="$JAVA_HOME/bin:$PATH"
//   analyzeHeadless ghidra_project MMU -process -noanalysis \
//     -scriptPath tools -postScript DecompileTargets.java

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.util.task.ConsoleTaskMonitor;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;

public class DecompileTargets extends GhidraScript {
    @Override
    public void run() throws Exception {
        File targets = new File("tools/decompile_targets.txt");
        if (!targets.exists()) {
            println("Missing tools/decompile_targets.txt");
            return;
        }
        File outDir = new File("docs/decompiled");
        outDir.mkdirs();

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        ConsoleTaskMonitor monitor = new ConsoleTaskMonitor();

        try (BufferedReader reader = new BufferedReader(new FileReader(targets))) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) continue;
                String token = line.split("\\s+")[0].replace("0x", "");
                Address addr = currentProgram.getAddressFactory().getAddress(token);
                Function func = getFunctionContaining(addr);
                if (func == null) {
                    println("NO FUNCTION at " + token);
                    continue;
                }
                DecompileResults result = decomp.decompileFunction(func, 90, monitor);
                if (!result.decompileCompleted()) {
                    println("FAILED " + token + ": " + result.getErrorMessage());
                    continue;
                }
                String name = func.getName();
                String c = "// " + name + " @ 0x" + token + "\n" + result.getDecompiledFunction().getC();
                Files.write(Paths.get("docs/decompiled/0x" + token + ".c"), c.getBytes());
                println("WROTE 0x" + token + "  " + name);
            }
        }
        decomp.dispose();
    }
}
