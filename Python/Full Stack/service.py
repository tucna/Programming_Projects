from flask import Flask, jsonify
from flask_cors import CORS

# Initialize the Flask app
app = Flask(__name__)

# Enable CORS for all routes, allowing requests from any origin.
# For production, you might want to specify a list of allowed origins.
# For this demo, using "*" is a good starting point.
CORS(app)

# Mock user database
users = [
    {"id": 1, "name": "Alice", "email": "alice@mail.com"},
    {"id": 2, "name": "Bob", "email": "bob@mail.com"}
]

@app.route("/users", methods=["GET"])
def get_users():
    """
    Returns a list of all users.
    """
    return jsonify(users)

@app.route("/users/<int:user_id>", methods=["GET"])
def get_user(user_id):
    """
    Returns a single user by their ID.
    """
    user = next((u for u in users if u["id"] == user_id), None)
    if user:
        return jsonify(user)
    return jsonify({"message": "User not found"}), 404

if __name__ == "__main__":
    # The debug flag should be set to False in a production environment.
    app.run(host="0.0.0.0", port=5001, debug=True)
