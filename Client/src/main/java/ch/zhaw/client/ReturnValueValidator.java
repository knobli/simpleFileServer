package ch.zhaw.client;

import java.lang.annotation.Annotation;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

public class ReturnValueValidator {
	
	public static boolean validateReturnValue(ReturnValue returnValue){
		ActionType action = returnValue.getAction();
		List<String> expectedResults = new ArrayList<String>();
		for(Field field : Protocol.class.getFields()){
			for(Annotation an : field.getAnnotations()){
				if(an instanceof ProtocolAnnotation){
					ProtocolAnnotation protAn = (ProtocolAnnotation) an;
					if(protAn.action() == action){
						try {
							expectedResults.add((String) field.get(null));
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
						
			}
		}
		for(String expectedResult : expectedResults){
			if(returnValue.getRetrunValue().matches(expectedResult)){
				return true;
			}
		}
		System.out.println("'" + returnValue.getRetrunValue() + "' does not match to one of those " + expectedResults);
		return false;
	}
}
