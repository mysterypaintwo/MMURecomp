// Ghidra headless post-script: find functions that reference the string
// addresses listed in tools/string_xrefs.txt, decompile them, and write one
// file per referencing function to docs/decompiled_xref/.
//
// Run with:
//   export JAVA_HOME=/opt/homebrew/opt/openjdk@25/libexec/openjdk.jdk/Contents/Home
//   export PATH="$JAVA_HOME/bin:$PATH"
//   analyzeHeadless ghidra_project MMU -process -noanalysis \
//     -scriptPath tools -postScript FindStringXrefs.java

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.util.task.ConsoleTaskMonitor;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.LinkedHashSet;
import java.util.Set;

public class FindStringXrefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        File targets = new File("tools/string_xrefs.txt");
        if (!targets.exists()) {
            println("Missing tools/string_xrefs.txt");
            return;
        }
        File outDir = new File("docs/decompiled_xref");
        outDir.mkdirs();
        File index = new File("docs/decompiled_xref/index.txt");

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        ConsoleTaskMonitor monitor = new ConsoleTaskMonitor();
        StringBuilder summary = new StringBuilder();

        try (BufferedReader reader = new BufferedReader(new FileReader(targets));
             PrintWriter indexWriter = new PrintWriter(index)) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) continue;
                String[] parts = line.split("\\s+");
                String label = parts[0];
                String token = parts[1].replace("0x", "");
                Address stringAddr = currentProgram.getAddressFactory().getAddress(token);
                Set<Function> funcs = new LinkedHashSet<>();
                for (Reference ref : getReferencesTo(stringAddr)) {
                    Function func = getFunctionContaining(ref.getFromAddress());
                    if (func != null) funcs.add(func);
                }
                summary.append(label).append(" 0x").append(token)
                       .append(" refs=").append(funcs.size()).append('\n');
                indexWriter.println(label + " 0x" + token + " refs=" + funcs.size());
                for (Function func : funcs) {
                    String funcToken = func.getEntryPoint().toString().replace(":", "");
                    while (funcToken.length() < 8) funcToken = "0" + funcToken;
                    indexWriter.println("  " + func.getName() + " @ 0x" + funcToken);
                    DecompileResults result = decomp.decompileFunction(func, 90, monitor);
                    if (!result.decompileCompleted()) {
                        println("FAILED " + label + " " + funcToken + ": " + result.getErrorMessage());
                        continue;
                    }
                    String c = "// " + label + " " + func.getName() + " @ 0x" + funcToken + "\n"
                             + result.getDecompiledFunction().getC();
                    Files.write(Paths.get("docs/decompiled_xref/" + label + "_0x" + funcToken + ".c"),
                                c.getBytes());
                    println("WROTE " + label + " 0x" + funcToken + " " + func.getName());
                }
            }
        }
        print(summary.toString());
        decomp.dispose();
    }
}
