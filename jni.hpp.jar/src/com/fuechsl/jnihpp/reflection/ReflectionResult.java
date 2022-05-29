package com.fuechsl.jnihpp.reflection;

import java.lang.reflect.Method;
import java.lang.reflect.Field;

/***
 * Result of a class reflection run
 * @author Ludwig Fuechsl
 */
public class ReflectionResult {
	/***
	 * Class identification
	 */
	public Class<?> cls = null;
	
	/***
	 * Superclass of this class
	 */
	public Class<?> supercls = null;
	
	/***
	 * Interfaces implement by this class
	 */
	public Class<?>[] interfaces = null;
	
	/***
	 * Array of methods
	 */
	public Method[] methods = null;
	
	/***
	 * Array of fields
	 */
	public Field[] fields = null;
	
	/***
	 * Get the name of the class that is reflected by this instance
	 * @return Name of reflected class (or null)
	 */
	public String GetClassName() {
		return (cls == null ? null : cls.getName());
	}
	
	/***
	 * Get the name of the superclass
	 * @return Name of reflected class's superclass (or null)
	 */
	public String GetSuperClassName() {
		return (supercls == null ? null : supercls.getName());
	}
	
	/***
	 * Get the implemented interface count
	 * @return The number of interfaces that this class is implementing
	 */
	public int GetInterfaceCount() {
		return (interfaces == null ? 0 : interfaces.length);
	}
	
	/***
	 * Get the name of the interface at index 
	 * @param index Interface index (>= 0 && < GetInterfaceCount(...) )
	 * @return Name of the implementd interface or null
	 */
	public String GetInterfaceName(int index) {
		return (index >= 0 && index < GetInterfaceCount() ? interfaces[index].getName() : null);
	}
	
	/***
	 * Get the number of methods implemented in this class
	 * @return Number of methods
	 */
	public int GetMethodCount() {
		return (methods == null ? 0 : methods.length);
	}
	
	/***
	 * Retrieve the name of the method
	 * @param index Index of method to be retrieved
	 * @return Method name as string or null
	 */
	public String GetMethodName(int index) {
		return (index >= 0 && index < GetMethodCount() ? methods[index].getName() : null);
	}
	
	/***
	 * Retrieve the jni signature of the method
	 * @param index Index of method to be retrieved
	 * @return Method signature as string or null
	 * @throws IllegalAccessException 
	 * @throws IllegalArgumentException 
	 */
	public String GetMethodSignature(int index) throws IllegalArgumentException, IllegalAccessException, NoSuchFieldException {
		// Get field and make accessible
		Field fd = Method.class.getDeclaredField("signature");
		fd.setAccessible(true);
		
		// Get jni signature
		return (index >= 0 && index < GetMethodCount() && fd != null ? (String)fd.get(methods[index]) : null);
	}
	
	/***
	 * Returns the number of fields declared in the reflected class
	 * @return The numbers of fields
	 */
	public int GetFieldCount() {
		return (fields == null ? 0 : fields.length);
	}
	
	/***
	 * Get the name of a reflected field
	 * @param index Index of the field to get the name
	 * @return Name of the fields
	 */
	public String GetFieldName(int index) {
		return (index >= 0 && index < GetFieldCount() ? fields[index].getName() : null);
	}
	
	/***
	 * Get the typename / signature of the field
	 * @param index Index of field to be queried
	 * @return Typename 
	 */
	public String GetFieldSignature(int index) {
		return (index >= 0 && index < GetFieldCount() ? fields[index].getGenericType().getTypeName() : null);
	}
}
