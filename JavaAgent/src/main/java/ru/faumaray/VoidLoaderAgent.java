package ru.faumaray;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.instrument.Instrumentation;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Set;

/**
 * VoidLoader Java Agent Wrapper
 * 
 * This is OPTIONAL - the C++ agent loads via Agent_OnLoad which happens
 * BEFORE any Java code runs. This wrapper is only needed if you want
 * to load the agent as a Java agent instead of a native JVMTI agent.
 * 
 * For native loading (recommended), use: -agentpath:/path/to/libVoidLoader_agent.so
 * For Java agent loading, use: -javaagent:VoidLoader-agent.jar
 */
public class VoidLoaderAgent {
    private static final String LIBRARY_NAME_LINUX = "libVoidLoader.so";
    private static final String LIBRARY_NAME_WINDOWS = "libVoidLoader.dll";
    private static final String LIBRARY_NAME_MAC = "libVoidLoader.dylib";
    private static final String TEMP_PREFIX = "VoidLoader_";
    private static File extractedLibrary = null;
    
    // Native method declarations (bridge to C++)
    private static native void nativeAgentStart();
    private static native void nativeAgentInit(String args, Instrumentation inst);
    private static native void nativeCleanup();
    
    static {
        System.out.println("[VoidLoader/Java] Java agent wrapper initializing...");
        loadNativeLibrary();
    }
    
    private static String getLibraryName() {
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            return LIBRARY_NAME_WINDOWS;
        } else if (os.contains("mac")) {
            return LIBRARY_NAME_MAC;
        } else {
            return LIBRARY_NAME_LINUX;
        }
    }
    
    private static void loadNativeLibrary() {
        try {
            // Try system path first
            try {
                System.loadLibrary("VoidLoader_agent");
                System.out.println("[VoidLoader/Java] Loaded from system library path");
                return;
            } catch (UnsatisfiedLinkError e) {
                System.out.println("[VoidLoader/Java] System library not found, extracting from JAR...");
            }
            
            // Extract from JAR
            extractAndLoadLibrary();
            
        } catch (Exception e) {
            System.err.println("[VoidLoader/Java] CRITICAL: Failed to load native library!");
            e.printStackTrace();
            System.err.println("\n" +
                "========================================\n" +
                "RECOMMENDED: Use native agent loading instead!\n" +
                "Add to JVM args: -agentpath:/path/to/libVoidLoader_agent.so\n" +
                "This loads BEFORE any Java code runs.\n" +
                "========================================\n");
        }
    }
    
    private static void extractAndLoadLibrary() throws IOException {
        String libraryName = getLibraryName();
        
        // Get library from JAR resources
        InputStream libStream = VoidLoaderAgent.class.getResourceAsStream("/" + libraryName);
        if (libStream == null) {
            throw new IOException("Native library not found in JAR: " + libraryName);
        }
        
        // Create temp directory
        Path tempDir = Files.createTempDirectory(TEMP_PREFIX);
        extractedLibrary = tempDir.resolve(libraryName).toFile();
        extractedLibrary.deleteOnExit();
        tempDir.toFile().deleteOnExit();
        
        // Extract library
        try (InputStream in = libStream;
             OutputStream out = new FileOutputStream(extractedLibrary)) {
            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
        }
        
        // Set executable permissions (Unix-like systems)
        try {
            Set<PosixFilePermission> perms = Files.getPosixFilePermissions(extractedLibrary.toPath());
            perms.add(PosixFilePermission.OWNER_EXECUTE);
            perms.add(PosixFilePermission.GROUP_EXECUTE);
            perms.add(PosixFilePermission.OTHERS_EXECUTE);
            Files.setPosixFilePermissions(extractedLibrary.toPath(), perms);
        } catch (UnsupportedOperationException e) {
            // Windows - use setExecutable
            extractedLibrary.setExecutable(true, false);
        }
        
        // Load the library
        System.load(extractedLibrary.getAbsolutePath());
        System.out.println("[VoidLoader/Java] Loaded from JAR: " + extractedLibrary.getAbsolutePath());
    }
    
    /**
     * Java agent entry point - called when using -javaagent
     * WARNING: This loads AFTER some Java classes have already loaded!
     * For best results, use -agentpath instead.
     */
    public static void premain(String args, Instrumentation inst) {
        System.out.println("\n========================================");
        System.out.println("[VoidLoader/Java] Java agent premain called");
        System.out.println("[VoidLoader/Java] WARNING: Some classes may have already loaded!");
        System.out.println("[VoidLoader/Java] For best results, use: -agentpath:/path/to/libVoidLoader_agent.so");
        System.out.println("========================================\n");
        
        try {
            nativeAgentStart();
            nativeAgentInit(args != null ? args : "", inst);
            System.out.println("[VoidLoader/Java] Agent initialized - delegating to Minecraft");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("[VoidLoader/Java] Native methods not available!");
            System.err.println("[VoidLoader/Java] The C++ agent may not have loaded correctly.");
            e.printStackTrace();
        } catch (Throwable t) {
            System.err.println("[VoidLoader/Java] Error during initialization:");
            t.printStackTrace();
        }
    }
    
    /**
     * Dynamic attach entry point
     */
    public static void agentmain(String args, Instrumentation inst) {
        System.out.println("[VoidLoader/Java] Dynamic attach not fully supported");
        premain(args, inst);
    }
    
    /**
     * Cleanup on shutdown
     */
    public static void cleanup() {
        try {
            if (extractedLibrary != null && extractedLibrary.exists()) {
                extractedLibrary.delete();
            }
            nativeCleanup();
        } catch (Throwable t) {
            // Ignore cleanup errors
        }
    }
    
    // Shutdown hook
    static {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            cleanup();
        }, "VoidLoader-Cleanup"));
    }
}