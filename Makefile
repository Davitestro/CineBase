.PHONY: run

run:
	@echo "🎬 Запуск CineVibe..."
	@cd frontend && npm run dev &
	@. .venv/bin/activate && python3 backend/main.py
