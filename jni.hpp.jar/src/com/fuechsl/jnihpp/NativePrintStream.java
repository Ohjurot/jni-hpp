package com.fuechsl.jnihpp;

import java.io.OutputStream;
import java.io.PrintStream;

/***
 * PrintStream class that will output to a native function
 * @author Ludwig Fuechsl
 */
public class NativePrintStream extends PrintStream {
	public NativeStreamType type = NativeStreamType.OutputStream;
	public long instanceid = 0; 
	
	/***
	 * Default constructor for streams
	 * @param out Stream
	 */
	public NativePrintStream(OutputStream out) {		
		// Init superclass that will auto flush
		super(out, true);
	}
	
	@Override
    public void print(String s){
		// check for nullptr (YES A POINTER IN JAVA)
		if(instanceid != 0){
			// Redirect according to stream type
			if(type == NativeStreamType.OutputStream)
				NStdOut(instanceid, s);
			else if(type == NativeStreamType.ErrorStream)
				NStdErr(instanceid, s);
		}
		else {
			// Use default
			super.print(s);
		}
    }
	
	/***
	 * Native std::cout (Can be hooked in a custom jni::JavaVM implementation)
	 * @param message Message to print
	 */
	public static native void NStdOut(long instanceid, String message);
	/***
	 * Native std::cerr (Can be hooked in a custom jni::JavaVM implementation)
	 * @param message Message to print
	 */
	public static native void NStdErr(long instanceid, String message);
}
