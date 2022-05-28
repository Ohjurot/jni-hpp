package com.fuechsl.jnihpp;

import java.io.IOException;
import java.io.InputStream;
import java.util.Objects;

/***
 * Reading redirect from c++
 * @author Ludwig Fuechsl
 */
public class NativeInputStream extends InputStream {
	// Don't touch this
	private long instanceID = 0;
	
	/***
	 * Constructor
	 * @param instanceID Pointer to JavaVM
	 */
	public NativeInputStream(long instanceID){
		this.instanceID = instanceID;
	}
	
	@Override
	public int read() throws IOException {
		return NReadByte(instanceID);
	}
	
	@Override
	public int read(byte[] b, int off, int len) throws IOException {
		Objects.checkFromIndexSize(off, len, b.length);
		int nread = 0;
		
		if(len > 0)
		{
			// Read first char blocking
			int val1 = read();
			if(val1 != -1)
			{
				b[off] = (byte)val1;
				int i = 1;
				for(; i < len; i++)
				{
					// Stop when no data is available
					if(available() == 0)
						break;
					
					// Read
					int vali = read();
					
					// Stop on EOF
					if(vali == -1)
						break;
					
					// Store
					b[off + i] = (byte)vali;
				}
				nread = i;
			}
		}
		
		return nread;
	}

	@Override
	public int available() throws IOException {
		return NCanRead(instanceID) ? 1 : 0;
	}
	
	@Override
	public long skip(long n) throws IOException {
		// Custom fast skip
		long remaining = n;
        for (;remaining > 0; remaining--) {
            int nr = read();
            if (nr < 0) {
                break;
            }
        }
        return n - remaining;
	}
	
	
	/**
	 * Native read function
	 * @param instanceID Pointer of JavaVM instance
	 * @return Byte that was read or -1
	 */
	private static native int NReadByte(long instanceID);
	
	/***
	 * Checks if std::cin has chars available
	 * @param instanceID Pointer of JavaVM instance
	 * @return True when a char can be read
	 */
	private static native boolean NCanRead(long instanceID);
}
