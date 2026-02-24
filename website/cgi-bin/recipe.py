#!/usr/bin/env python3

import urllib.request, json, sys, html

sys.stdout.write("Content-Type: text/html\r\n\r\n")
sys.stdout.flush()

try:
    res = urllib.request.urlopen("https://www.themealdb.com/api/json/v1/1/random.php")
    data = json.loads(res.read())
    meal = data['meals'][0]

    name = meal.get('strMeal', 'Unknown Meal')
    category = meal.get('strCategory', 'N/A')
    area = meal.get('strArea', 'N/A')
    image = meal.get('strMealThumb', '')
    video = meal.get('strYoutube', '#')

    # Build ingredient list safely
    ingredients = []
    for i in range(1, 21):
        ing = meal.get(f"strIngredient{i}")
        meas = meal.get(f"strMeasure{i}")
        if ing and ing.strip():
            meas = meas.strip() if meas else ''
            ingredients.append(f"{meas} {ing}".strip())

    # Full HTML block
    sys.stdout.write("<div style='padding:10px;border-radius:5px;color:rgb(238, 247, 255);font-family: Verdana, Geneva, Tahoma, sans-serif;'>")
    sys.stdout.write(f"<h2>{name}</h2>")
    sys.stdout.write(f"<p><strong>Category:</strong> {category} | <strong>Origin:</strong> {area}</p>")
    if image:
        sys.stdout.write(f"<img src='{image}' alt='{name}' style='width: 300px;border-radius:8px;'>")
    if ingredients:
        sys.stdout.write("<h4>Ingredients:</h4><ul>")
        for item in ingredients:
            sys.stdout.write(f"<li>{item}</li>")
        sys.stdout.write("</ul>")
    if video and video != '#':
        sys.stdout.write(f"<a style='text-decoration:none;background-color:rgb(108, 145, 194);color:white;padding:10px;border:none;border-radius:4px;cursor:pointer;' href='{video}' target='_blank'>Watch video</a>")
    sys.stdout.write("<br><br>")
    ingredient_string = "\n".join(ingredients)  # Use newline
    escaped_ingredients = html.escape(ingredient_string)  # Avoid HTML issues

    sys.stdout.write(f"""
		<form onsubmit="addIngredients(`{escaped_ingredients}`); return false;">
			<button type="submit" style="margin-top:10px;background-color:rgb(108, 145, 194);color:white;padding:10px;border:none;border-radius:4px;cursor:pointer;">Add Ingredients to Shopping List</button>
		</form>
    """)
    sys.stdout.write("</div>")
    sys.stdout.flush()

except Exception as e:
    sys.stdout.write(f"<p>Error fetching recipe: {str(e)}</p>")
    sys.stdout.flush()




# #!/usr/bin/env python3

# import urllib.request
# import json

# print("Content-Type: text/html\r\n")

# try:
#     response = urllib.request.urlopen("https://www.themealdb.com/api/json/v1/1/random.php")
#     data = json.loads(response.read())

#     meal = data['meals'][0]
#     name = meal['strMeal']
#     category = meal['strCategory']
#     area = meal['strArea']
#     instructions = meal['strInstructions']
#     image = meal['strMealThumb']
#     video = meal['strYoutube']

#     print("<div style='padding: 10px; background: #fff; border-radius: 5px;'>")
#     print("<h3>{}</h3>".format(name))
#     print("<p><strong>Category:</strong> {} | <strong>Origin:</strong> {}</p>".format(category, area))
#     print("<img src='{}' alt='{}' style='width: 200px;'>".format(image, name))
#     print("<p>{}</p>".format(instructions[:300] + "..."))
#     print("<a href='{}' target='_blank'>Watch video</a>".format(video))
#     print("</div>")

# except Exception as e:
#     print("<p>Error fetching recipe: {}</p>".format(str(e)))



# #!/usr/bin/env python3

# import sys
# import urllib.request
# import json

# print("Content-Type: text/html\r\n")

# try:
#     res = urllib.request.urlopen("https://www.themealdb.com/api/json/v1/1/random.php")
#     data = json.loads(res.read())
#     meal = data["meals"][0]

#     title = meal["strMeal"]
#     instructions = meal["strInstructions"][:300]
#     image_url = meal["strMealThumb"]

#     # Output formatted HTML
#     print(f"""
#     <div class="recipe">
#         <h2>{title}</h2>
#         <img src="{image_url}" alt="Meal image" class="recipe-img"/>
#         <p>{instructions}...</p>
#     </div>
#     """)

# except Exception as e:
#     print(f"<p>Error: {e}</p>")
