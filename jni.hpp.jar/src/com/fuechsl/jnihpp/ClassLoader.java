package com.fuechsl.jnihpp;

/***
 * Custom class loader that calls C++ (
 * @author Ludwig Fuechsl
 */
public class ClassLoader extends java.lang.ClassLoader {
	// Instance ID of the native jvm class to be called (aka. a pointer that you shall never touch!)
	private static long nativeInstanceId = 0;
	
	/***
	 * Constructor
	 */
	public ClassLoader(java.lang.ClassLoader classLoader){
		super(classLoader);
	}
	
	@Override
	protected Class<?> findClass(String name) throws ClassNotFoundException {
		Class<?> cls = null;
		
		// Do custom loading
		if(nativeInstanceId != 0){
			ClassLoadResult result = NLoadClassData(nativeInstanceId, name);
			if(result.loaded){
				// Craft class
				cls = defineClass(name, result.data, 0, result.data.length);
			}
		}
		
		// Throw if the class was not found
		if(cls == null) {
			throw new ClassNotFoundException("Failed to load the class using the default java classloader and the C++ Native interface");
		}
		
		return cls;
	}
	
	/***
	 * 
	 * @param instance Instance ID of the JavaVM instance object that shall receive the call
	 * @param name Name of the class that should be found
	 * @return ClassLoadResult containing the required data
	 */
	public static native ClassLoadResult NLoadClassData(long instance, String name);
}
