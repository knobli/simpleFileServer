package ch.zhaw.client;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Target( value = ElementType.FIELD )
@Retention(RetentionPolicy.RUNTIME)
public @interface ProtocolAnnotation {
	ActionType action();
}
