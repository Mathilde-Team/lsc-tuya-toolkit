// Exporte les références vers des fonctions depuis des DONNÉES (tables de pointeurs :
// dispatch des commandes Tuya, callbacks). Sortie TSV : adresse_donnée, fonction.
// @category Firmware
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import java.io.PrintWriter;

public class ExportTables extends GhidraScript {
    @Override
    public void run() throws Exception {
        PrintWriter w = new PrintWriter(getScriptArgs()[0]);
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext()) {
            Function f = it.next();
            for (Reference r : currentProgram.getReferenceManager().getReferencesTo(f.getEntryPoint())) {
                if (currentProgram.getFunctionManager().getFunctionContaining(r.getFromAddress()) == null) {
                    w.println(r.getFromAddress() + "\t" + f.getName());
                }
            }
        }
        w.close();
        println("tables exported");
    }
}
