package nl.hetorus.craftinginterpreters.tools;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.List;

public class GenerateAst {
    // define variables for the indents
    private static final int indentSize = 4;
    private static final String indent1 = " ".repeat(indentSize);
    private static final String indent2 = indent1.repeat(2);
    private static final String indent3 = indent1.repeat(3);

    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("Usage: GenerateAst <output-directory>");
            System.exit(64);
        }
        String outputDir = args[0];
        defineAst(outputDir, "Expr", Arrays.asList(
                "Assign   : Token name, Expr value",
                "Binary   : Expr left, Token operator, Expr right",
                "Call     : Expr callee, Token paren, List<Expr> arguments",
                "Get      : Expr object, Token name",
                "Grouping : Expr expression",
                "Literal  : Object value",
                "Logical  : Expr left, Token operator, Expr right",
                "Set      : Expr object, Token name, Expr value",
                "Super    : Token keyword, Token method",
                "This     : Token keyword",
                "Unary    : Token operator, Expr right",
                "Variable : Token name"));

        defineAst(outputDir, "Stmt", Arrays.asList(
                "Block      : List<Stmt> statements",
                "Class      : Token name, Expr.Variable superclass, List<Stmt.Function> methods",
                "Expression : Expr expression",
                "Function   : Token name, List<Token> params, List<Stmt> body",
                "If         : Expr condition, Stmt thenBranch, Stmt elseBranch",
                "Print      : Expr expression",
                "Return     : Token keyword, Expr value",
                "Var        : Token name, Expr initializer",
                "While      : Expr condition, Stmt body"));
    }

    private static void defineAst(String outputDir, String baseName, List<String> types) throws IOException {
        String path = outputDir + "/" + baseName + ".java";
        // make sure the directory and file exists
        File file = new File(outputDir);
        file.mkdirs();
        // create the writer
        PrintWriter writer = new PrintWriter(path, "UTF-8");

        // create the package, list the imports and create the abstract class
        writer.println("package nl.hetorus.craftinginterpreters.lox;");
        writer.println();
        writer.println("import java.util.List;");
        writer.println();
        writer.println("abstract class " + baseName + " {");

        defineVisitor(writer, baseName, types);

        // the AST classees
        boolean isFirst = true;
        for (String type : types) {
            String className = type.split(":")[0].trim();
            String fields = type.split(":")[1].trim();
            defineType(writer, baseName, className, fields, isFirst);
            isFirst = false;
        }

        // the base accept() method
        writer.println();
        writer.println(indent1 + "abstract <R> R accept(Visitor<R> visitor);");

        // close the abstract class and close the writer
        writer.println("}");
        writer.close();
    }

    private static void defineType(PrintWriter writer, String baseName, String className, String fieldList,
            boolean isFirst) {
        // add whitespace after the previous class (if not isFirst)
        if (!isFirst)
            writer.println();

        // add the definition of the static class
        writer.println(indent1 + "static class " + className + " extends " + baseName + " {");

        // fields
        String[] fields = fieldList.split(", ");
        for (String field : fields) {
            writer.println(indent2 + "final " + field.trim() + ";");
        }

        // constructor
        writer.println();
        writer.println(indent2 + className + "(" + fieldList + ") {");
        // store parameters in the fields
        for (String field : fields) {
            String name = field.trim().split(" ")[1].trim();
            writer.println(indent3 + "this." + name + " = " + name + ";");
        }
        writer.println(indent2 + "}");

        // visitor pattern
        writer.println();
        writer.println(indent2 + "@Override");
        writer.println(indent2 + "<R> R accept(Visitor<R> visitor) {");
        writer.println(indent3 + "return visitor.visit" + className + baseName + "(this);");
        writer.println(indent2 + "}");

        writer.println(indent1 + "}");
    }

    private static void defineVisitor(PrintWriter writer, String baseName, List<String> types) {
        writer.println(indent1 + "interface Visitor<R> {");

        boolean isFirst = true;
        for (String type : types) {
            if (!isFirst)
                writer.println();
            String typeName = type.split(":")[0].trim();
            writer.print(indent2 + "R visit" + typeName + baseName + "(");
            writer.println(typeName + " " + baseName.toLowerCase() + ");");
            isFirst = false;
        }

        writer.println(indent1 + "}");
        writer.println();
    }
}
