package com.fuechsl.jnihpp.reflection;

/***
 * Class that statically reflects a class by its name 
 * @author Ludwig Fuechsl
 */
public final class ClassReflector {
	
	/***
	 * Reflect class by its name
	 * @param name Name of the class
	 * @return Reflected class
	 */
	public static ReflectionResult ReflectClass(String name) throws ClassNotFoundException{
		// Create class by name and run reflection
		Class<?> cls = Class.forName(name.replaceAll("/", "."));
		return ReflectClass(cls);
	}
	
	/***
	 * Will reflect the given class
	 * @param cls Class to be reflected
	 * @return Result of reflection
	 */
	public static ReflectionResult ReflectClass(Class<?> cls){
		ReflectionResult result = new ReflectionResult();
		
		// Set members
		result.cls = cls;
		result.supercls = cls.getSuperclass();
		result.interfaces = cls.getInterfaces();
		result.fields = cls.getDeclaredFields();
		result.methods = cls.getDeclaredMethods();
		
		// Return final result
		return result;
	}
}
