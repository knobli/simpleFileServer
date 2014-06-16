package ch.zhaw.client;

public class ReturnValue {
	
	private ActionType action;
	private String retrunValue;

	public ReturnValue() {
		super();
	}

	
	public ReturnValue(String retrunValue, ActionType action) {
		super();
		this.retrunValue = retrunValue;
		this.action = action;
	}

	public String getRetrunValue() {
		return retrunValue;
	}

	public void setRetrunValue(String retrunValue) {
		this.retrunValue = retrunValue;
	}

	public ActionType getAction() {
		return action;
	}

	public void setAction(ActionType action) {
		this.action = action;
	}

}
