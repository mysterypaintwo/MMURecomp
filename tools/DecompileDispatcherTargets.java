// Ghidra headless post-script for the named-state dispatcher recovered around
// the global at 0x6c55cc. Writes results to docs/decompiled/.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.util.task.ConsoleTaskMonitor;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.nio.file.Files;
import java.nio.file.Paths;

public class DecompileDispatcherTargets extends GhidraScript {
    @Override
    public void run() throws Exception {
        File targets = new File("tools/decompile_dispatcher_targets.txt");
        if (!targets.exists()) {
            println("Missing tools/decompile_dispatcher_targets.txt");
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
                String token = line.replace("0x", "");
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
                String c = "// " + func.getName() + " @ 0x" + token + "\n"
                        + result.getDecompiledFunction().getC();
                Files.write(Paths.get("docs/decompiled/0x" + token + ".c"), c.getBytes());
                println("WROTE 0x" + token + "  " + func.getName());
            }
        }
        decomp.dispose();
    }
}
