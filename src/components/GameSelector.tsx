import { motion } from "framer-motion";
import { cn } from "@/lib/utils";
import type { GameType } from "@/pages/Index";

interface GameSelectorProps {
  selectedGame: GameType;
  onGameChange: (game: GameType) => void;
}

const games: { id: GameType; name: string; description: string; colorClass: string }[] = [
  {
    id: "loto",
    name: "Loto",
    description: "5 numéros + 1 chance",
    colorClass: "bg-loto hover:bg-loto/90 text-loto-foreground",
  },
  {
    id: "euromillions",
    name: "EuroMillions",
    description: "5 numéros + 2 étoiles",
    colorClass: "bg-euromillions hover:bg-euromillions/90 text-euromillions-foreground",
  },
  {
    id: "eurodreams",
    name: "EuroDreams",
    description: "6 numéros + 1 Dream",
    colorClass: "bg-eurodreams hover:bg-eurodreams/90 text-eurodreams-foreground",
  },
  {
    id: "crescendo",
    name: "Crescendo",
    description: "5 numéros",
    colorClass: "bg-crescendo hover:bg-crescendo/90 text-crescendo-foreground",
  },
];

export const GameSelector = ({ selectedGame, onGameChange }: GameSelectorProps) => {
  return (
    <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
      {games.map((game, index) => (
        <motion.button
          key={game.id}
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ delay: index * 0.1 }}
          onClick={() => onGameChange(game.id)}
          className={cn(
            "relative p-4 rounded-xl transition-all duration-200",
            "border-2 shadow-lg",
            selectedGame === game.id
              ? cn(game.colorClass, "border-transparent scale-105")
              : "bg-card border-border hover:border-primary/50"
          )}
        >
          {selectedGame === game.id && (
            <motion.div
              layoutId="selectedGame"
              className="absolute inset-0 rounded-xl"
              transition={{ type: "spring", bounce: 0.2, duration: 0.6 }}
            />
          )}
          <div className="relative z-10">
            <h3 className="font-bold text-lg">{game.name}</h3>
            <p className={cn(
              "text-sm mt-1",
              selectedGame === game.id ? "opacity-90" : "text-muted-foreground"
            )}>
              {game.description}
            </p>
          </div>
        </motion.button>
      ))}
    </div>
  );
};
