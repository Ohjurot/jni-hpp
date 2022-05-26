package com.fuechsl.jnihpp;

import java.io.PrintStream;

/***
 * Class that is responsible for setting up all java sided things
 * @author Ludwig Fuechsl
 */
public class Setup {
	/***
	 * Runs the static setup required to hook into the jni.hpp JavaVM
	 * @param instanceID JavaVM instance id
	 */
	public static void RunSetup(long instanceID){
		// Redirect stdout
		PrintStream defaultOut = System.out;
		NativePrintStream nativeStdout = new NativePrintStream(defaultOut);
		nativeStdout.type = NativeStreamType.OutputStream;
		nativeStdout.instanceid = instanceID;
		System.setOut(nativeStdout);
		
		// Redirect stderr
		PrintStream defaultErr = System.err;
		NativePrintStream nativeStderr = new NativePrintStream(defaultErr);
		nativeStderr.type = NativeStreamType.ErrorStream;
		nativeStderr.instanceid = instanceID;
		System.setErr(nativeStderr);
	}
}
